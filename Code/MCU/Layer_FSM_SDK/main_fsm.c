/*! ***************************************************************************
 * \brief  main_fsm.c — Layer 5 FSM Implementation
 * \file   main_fsm.c
 *
 * State machine: IDLE → LOCATOR → PUZZLE → UNLOCK_NEXT → FINAL
 * Plus super-states: ADMIN, FAULT
 *
 * Event flow: scan_task → event_dispatcher → main_fsm → outputs
 *****************************************************************************/
#include "cmsis_compiler.h"
#include "main_fsm.h"
#include "log.h"
#include "nvm_drv.h"
#include "lock_drv.h"
#include "feedback_drv.h"
#include "button_drv.h"
#include "extio_drv.h"        /* Layer 4 external LEDs + buttons (item 1/3) */
#include "ble_drv.h"
#include "location_engine.h"
#include "config_store.h"
#include "puzzle_registry.h"   /* Khanh's games behind the plugin contract */
#include "display_drv.h"       /* Layer 4 display (wraps tft_lcd) */
#include "v_screen.h"          /* Layer 6 V_SCREEN text rows */
#include "rssi_bar.h"          /* reset rolling avg on target change */
#include <string.h>

/* -------------------------------------------------------------------------
 * Timestamp source
 * ---------------------------------------------------------------------- */
extern volatile uint32_t ms;

/* -------------------------------------------------------------------------
 * Event queue (16 slots, single-producer/single-consumer in bare-metal)
 * ---------------------------------------------------------------------- */
static fsm_event_t g_queue[FSM_QUEUE_SIZE];
static volatile uint8_t g_q_head = 0;
static volatile uint8_t g_q_tail = 0;

/* -------------------------------------------------------------------------
 * FSM state
 * ---------------------------------------------------------------------- */
static fsm_state_t g_state = FSM_IDLE;
static uint8_t     g_progress = 0;       /**< Rooms completed = route pos */
static uint8_t     g_current_room = 0;   /**< Current detected room (0-based index) */
static uint8_t     g_target_room = 0;    /**< Next room to reach (0-based index)    */
static uint32_t    g_state_enter_ms = 0; /**< Timestamp of last state entry */

/* Layer 7 model: route + room map. Borrowed pointer, not owned (may be NULL). */
static const config_store_t *g_cfg = 0;

/* Active puzzle plugin for the current PUZZLE state (NULL when not in PUZZLE) */
static const puzzle_plugin_t *g_active_puzzle = 0;

/* True while the ARMED state is for a GAME step (red LED → puzzle on PLAY);
 * false for a WAYPOINT step (green LED → advance route on PLAY). */
static bool g_armed_is_game = false;

/* -------------------------------------------------------------------------
 * Route helpers (Option C — data-driven order from config_store)
 *
 * The FSM no longer hardcodes "1,2,3,4,5". It asks the config route which
 * room index sits at each step. route_len() = how many stops the hunt has;
 * route_room_at(pos) = which room index (0-based) the kid must reach at
 * step `pos`. Admin can reorder or even repeat a room via config_set_route().
 * If no config is supplied, both fall back to a plain sequential identity.
 * ---------------------------------------------------------------------- */
static uint8_t route_len(void)
{
    if (g_cfg != 0)
    {
        uint8_t c = config_get_route_count(g_cfg);
        if (c > 0) return c;
    }
    return CONFIG_MAX_ROOMS;
}

static uint8_t route_room_at(uint8_t pos)
{
    if (g_cfg != 0)
    {
        uint8_t b = config_get_route_beacon(g_cfg, pos);
        if (b != 0xFF) return b;
    }
    return pos;  /* fallback: sequential 0,1,2,... */
}

/* -------------------------------------------------------------------------
 * State name table
 * ---------------------------------------------------------------------- */
static const char * const state_names[FSM_STATE_COUNT] = {
    "IDLE", "LOCATOR", "PUZZLE", "UNLOCK_NEXT", "FINAL", "ADMIN", "FAULT",
    "READY", "ARMED"
};

/* -------------------------------------------------------------------------
 * Internal: transition to a new state
 * ---------------------------------------------------------------------- */
static void transition(fsm_state_t new_state)
{
    if (new_state == g_state) return;

    LogWithNum(LOG_SYS_FSM, LOG_INFO, "transition: ", (int32_t)new_state);
    Log(LOG_SYS_FSM, LOG_INFO, state_names[new_state]);

    g_state = new_state;
    g_state_enter_ms = ms;

    /* On entering PUZZLE: start the game for the CURRENT route step (F2.1).
     * Role + puzzle live on the step (g_progress), so the same beacon can be a
     * game here and a waypoint elsewhere. */
    if (new_state == FSM_PUZZLE)
    {
        uint8_t pid = g_target_room;  /* fallback: beacon index == puzzle */
        if (g_cfg != 0)
        {
            pid = config_get_route_puzzle(g_cfg, g_progress);
        }
        g_active_puzzle = puzzle_registry_get(pid);
        if (g_active_puzzle != 0)
        {
            Log(LOG_SYS_FSM, LOG_INFO, g_active_puzzle->name);
            g_active_puzzle->start();   /* game draws its own full screen */
        }
    }
    else
    {
        g_active_puzzle = 0;  /* leaving PUZZLE — no active game */
    }

    /* On entering ARMED: light the matching steady LED inviting the button
     * press. RED solid for a game step, GREEN solid for a waypoint step.
     * Cleared by the state handler the instant the button fires. */
    if (new_state == FSM_ARMED)
    {
        extio_led_set(g_armed_is_game ? EXTIO_LED_ARM : EXTIO_LED_READY, true);
    }

    /* V_SCREEN: while hunting, what to show depends on the target's role.
     *  - GAME target → show Room/Next/Puzzle (the scan task keeps the bar).
     *  - WAYPOINT target → reset the HOTTER/COLDER screen; the scan task draws
     *    the warmth word (no game info on a transition beacon).
     * PUZZLE owns the screen itself; FINAL shows its own message. */
    if (new_state == FSM_LOCATOR || new_state == FSM_IDLE)
    {
        bool target_game = (g_cfg == 0) ||
            (config_get_route_type(g_cfg, g_progress) == BEACON_GAME);

        if (target_game)
        {
            const puzzle_plugin_t *next_p = puzzle_registry_get(
                (g_cfg != 0) ? config_get_route_puzzle(g_cfg, g_progress)
                             : g_target_room);
            v_screen_draw();
            v_screen_update(g_current_room, g_target_room,
                            (next_p != 0) ? next_p->name : "--");
        }
        else
        {
            v_screen_waypoint_reset();  /* warmth word redraws on next scan */
        }
    }

    /* Persist state transition to NVM (F7.2) */
    nvm_record_t rec;
    memset(&rec, 0, sizeof(rec));
    rec.room_id = g_current_room;
    rec.route_progress = g_progress;
    rec.timestamp_ms = ms;

    switch (new_state)
    {
    case FSM_LOCATOR:
        rec.outcome = NVM_OUTCOME_ENTERED;
        break;
    case FSM_UNLOCK_NEXT:
        rec.outcome = NVM_OUTCOME_PUZZLE_OK;
        break;
    case FSM_FINAL:
        rec.outcome = NVM_OUTCOME_UNLOCKED;
        break;
    case FSM_ADMIN:
        rec.outcome = NVM_OUTCOME_ADMIN;
        break;
    default:
        rec.outcome = NVM_OUTCOME_ENTERED;
        break;
    }

    nvm_drv_write(&rec);
}

/* -------------------------------------------------------------------------
 * API: Init
 * ---------------------------------------------------------------------- */

void fsm_init(const config_store_t *cfg)
{
    /* Borrow the route + room map (Option C). NULL → sequential fallback. */
    g_cfg = cfg;

    /* Initialize all subsystems */
    LogInit();
    Log(LOG_SYS_SYS, LOG_INFO, "FSM init start");

    nvm_drv_init();
    lock_drv_init();
    feedback_drv_init();
    button_drv_init();
    extio_drv_init();     /* external LEDs + START/PLAY buttons (item 1/3) */

    /* Layer 4/6: bring up the TFT and draw the static V_SCREEN labels */
    display_open();
    v_screen_draw();

    /* Check for persisted session (F7.3: resume after power cut) */
    nvm_record_t last;
    if (nvm_drv_read_latest(&last))
    {
        g_progress = last.route_progress;
        g_current_room = last.room_id;
        LogWithNum(LOG_SYS_SYS, LOG_INFO, "Resumed session, progress=", g_progress);
    }

    /* Start in READY (item 1): box configured + unplugged from PC, now waits
     * for the assistant to press the external START button before the hunt
     * begins. The external READY LED lights to invite that press. */
    g_state = FSM_READY;
    g_state_enter_ms = ms;
    lock_drv_close();
    extio_led_all_off();
    extio_led_set(EXTIO_LED_READY, true);   /* steady: "press START to begin" */

    /* Clear event queue */
    g_q_head = 0;
    g_q_tail = 0;

    Log(LOG_SYS_SYS, LOG_INFO, "FSM init complete");
}

/* -------------------------------------------------------------------------
 * API: Post event
 * ---------------------------------------------------------------------- */

bool fsm_post_event(const fsm_event_t *evt)
{
    if (evt == NULL) return false;

    uint8_t next = (g_q_head + 1) % FSM_QUEUE_SIZE;
    if (next == g_q_tail)
    {
        Log(LOG_SYS_FSM, LOG_WARN, "event queue full, dropped");
        return false;  /* queue full */
    }

    __disable_irq();
    g_queue[g_q_head] = *evt;
    g_q_head = next;
    __enable_irq();

    return true;
}

/* -------------------------------------------------------------------------
 * API: Process event (the state machine core)
 * ---------------------------------------------------------------------- */

bool fsm_process_event(void)
{
    if (g_q_tail == g_q_head) return false;  /* empty */

    /* Pop event */
    fsm_event_t evt = g_queue[g_q_tail];
    g_q_tail = (g_q_tail + 1) % FSM_QUEUE_SIZE;

    /* Update feedback state machine (non-blocking cues) */
    feedback_drv_update();

    /* --- State machine --- */
    switch (g_state)
    {
    /* ================================================================
     * READY: box powered/configured, external GREEN LED on, waiting for
     * the assistant to press the external START button (item 1).
     * ================================================================ */
    case FSM_READY:
        if (evt.type == EVT_START_SESSION)
        {
            Log(LOG_SYS_FSM, LOG_INFO, "START pressed — session begins");
            extio_led_set(EXTIO_LED_READY, false);  /* LED off on press */
            transition(FSM_IDLE);
        }
        else if (evt.type == EVT_ADMIN_ENTER)
        {
            extio_led_set(EXTIO_LED_READY, false);
            transition(FSM_ADMIN);
        }
        break;

    /* ================================================================
     * IDLE: Waiting for first beacon detection
     * ================================================================ */
    case FSM_IDLE:
        if (evt.type == EVT_BEACON_FOUND)
        {
            g_current_room = evt.data.beacon.room_id;
            g_target_room = route_room_at(g_progress);  /* route step 0 (or resumed) */
            LogWithNum(LOG_SYS_FSM, LOG_INFO, "First beacon, target=", g_target_room);
            transition(FSM_LOCATOR);
        }
        else if (evt.type == EVT_ADMIN_ENTER)
        {
            transition(FSM_ADMIN);
        }
        break;

    /* ================================================================
     * LOCATOR: Scanning, waiting to reach target room
     * ================================================================ */
    case FSM_LOCATOR:
        if (evt.type == EVT_BEACON_FOUND)
        {
            g_current_room = evt.data.beacon.room_id;

            /* Item 2: role lives on the route STEP. Both roles now "arm" with a
             * solid LED + button press; only the threshold, LED color and the
             * PLAY action differ.
             *   GAME     → red solid at ~2 m → PLAY starts the puzzle
             *   WAYPOINT → green solid at ~1 m → PLAY advances to next stop  */
            bool is_game = (g_cfg == 0) ||
                (config_get_route_type(g_cfg, g_progress) == BEACON_GAME);
            int8_t arm_th = is_game ? FSM_ARM_RSSI_DBM : FSM_ARM_RSSI_WP_DBM;

            if (g_current_room == g_target_room &&
                evt.data.beacon.rssi >= arm_th)
            {
                g_armed_is_game = is_game;
                Log(LOG_SYS_FSM, LOG_INFO, is_game
                    ? "Game beacon — press button to play"
                    : "Waypoint reached — press button for next");
                transition(FSM_ARMED);
            }
        }
        else if (evt.type == EVT_BEACON_LOST)
        {
            Log(LOG_SYS_SENSOR, LOG_WARN, "signal_low");
        }
        else if (evt.type == EVT_ADMIN_ENTER)
        {
            transition(FSM_ADMIN);
        }
        else if (evt.type == EVT_FAULT)
        {
            transition(FSM_FAULT);
        }
        break;

    /* ================================================================
     * ARMED: kid is close enough to the target beacon; the matching LED is
     * SOLID and we wait for the external button. On press:
     *   GAME step     → start the puzzle
     *   WAYPOINT step → advance to the next route stop (no game)
     * If the kid walks away (RSSI drops past the hysteresis edge), disarm.
     * ================================================================ */
    case FSM_ARMED:
        if (evt.type == EVT_START_GAME)
        {
            if (g_armed_is_game)
            {
                Log(LOG_SYS_FSM, LOG_INFO, "Button — starting puzzle");
                extio_led_set(EXTIO_LED_ARM, false);   /* red off */
                /* Flush proximity buffers — puzzle owns the screen now, and
                 * when it finishes the FSM will advance to a new target. */
                rssi_bar_reset();
                v_screen_waypoint_reset();
                transition(FSM_PUZZLE);
            }
            else
            {
                Log(LOG_SYS_FSM, LOG_INFO, "Button — waypoint cleared, next stop");
                extio_led_set(EXTIO_LED_READY, false);  /* green off */

                /* ---- FLUSH stale proximity data before switching target ----
                 * The rssi_bar ring + v_screen cache still hold "HOT" from the
                 * beacon we just left. Reset so the kid sees "COLD" instantly
                 * until the new target's signal fills the buffer. */
                rssi_bar_reset();
                v_screen_waypoint_reset();

                g_progress++;                           /* trail advances */
                if (g_progress >= route_len())
                {
                    transition(FSM_FINAL);
                }
                else
                {
                    g_target_room = route_room_at(g_progress);
                    LogWithNum(LOG_SYS_FSM, LOG_INFO,
                               "Next target room=", g_target_room);
                    transition(FSM_LOCATOR);
                }
            }
        }
        else if (evt.type == EVT_BEACON_FOUND &&
                 evt.data.beacon.rssi <
                   ((g_armed_is_game ? FSM_ARM_RSSI_DBM : FSM_ARM_RSSI_WP_DBM)
                    - FSM_ARM_HYSTERESIS))
        {
            /* Wandered out of range before pressing — disarm. */
            Log(LOG_SYS_FSM, LOG_INFO, "Moved away — disarm to LOCATOR");
            extio_led_all_off();
            transition(FSM_LOCATOR);
        }
        else if (evt.type == EVT_BEACON_LOST)
        {
            Log(LOG_SYS_FSM, LOG_INFO, "Signal lost — disarm to LOCATOR");
            extio_led_all_off();
            transition(FSM_LOCATOR);
        }
        else if (evt.type == EVT_ADMIN_ENTER)
        {
            extio_led_all_off();
            transition(FSM_ADMIN);
        }
        break;

    /* ================================================================
     * PUZZLE: Khanh's game active, driven via the puzzle plugin contract
     * Touch/button input is routed to the active puzzle's input(); the
     * main loop polls the puzzle's tick() and posts EVT_PUZZLE_SOLVED.
     * ================================================================ */
    case FSM_PUZZLE:
        if (evt.type == EVT_PUZZLE_SOLVED)
        {
            Log(LOG_SYS_FSM, LOG_INFO, "Puzzle solved!");
            feedback_cue_puzzle();  /* F3.2: 10 ms green LED */
            g_progress++;
            transition(FSM_UNLOCK_NEXT);
        }
        else if (evt.type == EVT_PUZZLE_FAIL)
        {
            Log(LOG_SYS_FSM, LOG_WARN, "Puzzle failed, retry");
            feedback_cue_error();  /* F2.3: 100 ms red + 500 Hz */
            /* Stay in PUZZLE state */
        }
        else if (evt.type == EVT_ADMIN_ENTER)
        {
            transition(FSM_ADMIN);
        }
        break;

    /* ================================================================
     * UNLOCK_NEXT: Puzzle solved, reveal next room (F2.2: within 500 ms)
     * ================================================================ */
    case FSM_UNLOCK_NEXT:
        /* Check if the whole route is done (route length, not a magic 5) */
        if (g_progress >= route_len())
        {
            Log(LOG_SYS_FSM, LOG_INFO, "All rooms complete! → FINAL");
            transition(FSM_FINAL);
        }
        else
        {
            /* ---- FLUSH stale proximity data before switching target ----
             * After puzzle solved, kid is still near the old beacon. Reset
             * so feedback starts "COLD" for the new target immediately. */
            rssi_bar_reset();
            v_screen_waypoint_reset();

            /* Advance to the next route step (data-driven order — Option C) */
            g_target_room = route_room_at(g_progress);
            LogWithNum(LOG_SYS_FSM, LOG_INFO, "Next target room=", g_target_room);
            transition(FSM_LOCATOR);
        }
        break;

    /* ================================================================
     * FINAL: All rooms done, open the lock! (F4.1: within 500 ms)
     * ================================================================ */
    case FSM_FINAL:
        if (g_state_enter_ms != 0)  /* First entry */
        {
            Log(LOG_SYS_FSM, LOG_CRIT, "FINAL — opening lock!");
            display_clear(RGB_BLACK);
            display_put_string(40, 100, "ALL ROOMS DONE!", RGB_GREEN, RGB_BLACK);
            display_put_string(60, 130, "Box unlocked!", RGB_YELLOW, RGB_BLACK);
            feedback_cue_final();    /* F3.6: 3 s celebration */
            lock_drv_open();         /* F4.1: drive lock open */
            g_state_enter_ms = 0;    /* Don't re-trigger */
        }
        /* Stay in FINAL until admin resets */
        if (evt.type == EVT_ADMIN_ENTER)
        {
            transition(FSM_ADMIN);
        }
        break;

    /* ================================================================
     * ADMIN: USB-CDC admin mode (F5.1–F5.4)
     * ================================================================ */
    case FSM_ADMIN:
        if (evt.type == EVT_ADMIN_EXIT)
        {
            Log(LOG_SYS_FSM, LOG_INFO, "Admin exit");
            lock_drv_close();
            /* Item 1: after exiting admin, return to READY so the assistant
             * must press START again before the next session begins. */
            extio_led_all_off();
            extio_led_set(EXTIO_LED_READY, true);
            transition(FSM_READY);
        }
        else if (evt.type == EVT_ADMIN_UNLOCK)
        {
            Log(LOG_SYS_FSM, LOG_INFO, "Admin unlock");
            lock_drv_open();  /* F4.4 */
        }
        break;

    /* ================================================================
     * FAULT: Error state
     * ================================================================ */
    case FSM_FAULT:
        Log(LOG_SYS_FSM, LOG_ERROR, "FAULT state — waiting for reset");
        /* Only admin can recover */
        if (evt.type == EVT_ADMIN_ENTER)
        {
            transition(FSM_ADMIN);
        }
        break;

    default:
        transition(FSM_FAULT);
        break;
    }

    return true;
}

/* -------------------------------------------------------------------------
 * API: Puzzle bridge (drives Khanh's games via the plugin contract)
 * ---------------------------------------------------------------------- */

void fsm_puzzle_poll(void)
{
    /* Only meaningful while a game is active in PUZZLE state */
    if (g_state != FSM_PUZZLE || g_active_puzzle == 0) return;

    /* Ask the active game: solved yet? (F2.2) */
    if (g_active_puzzle->tick())
    {
        fsm_event_t solved = { .type = EVT_PUZZLE_SOLVED, .timestamp = ms };
        fsm_post_event(&solved);
        g_active_puzzle = 0;  /* one solved event per puzzle */
    }
}

void fsm_puzzle_input(void)
{
    /* Route a touch/button event into the active game */
    if (g_state == FSM_PUZZLE && g_active_puzzle != 0)
    {
        g_active_puzzle->input();
    }
}

/* -------------------------------------------------------------------------
 * API: Getters
 * ---------------------------------------------------------------------- */

fsm_state_t fsm_get_state(void)
{
    return g_state;
}

const char *fsm_state_name(fsm_state_t state)
{
    if (state < FSM_STATE_COUNT) return state_names[state];
    return "UNKNOWN";
}

uint8_t fsm_get_progress(void)
{
    return g_progress;
}

bool fsm_target_is_game(void)
{
    /* Is the CURRENT route step a game beacon? (drives red vs green blink) */
    if (g_cfg == 0) return true;
    return (config_get_route_type(g_cfg, g_progress) == BEACON_GAME);
}

uint8_t fsm_get_target_room(void)
{
    return g_target_room;
}
