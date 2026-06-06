/*! ***************************************************************************
 * \brief  kidbytes_fsm — Layer 5 Integration (Active Object pattern)
 * \file   main.c
 *
 * This is the top-level firmware that integrates ALL layers:
 *   - scan_task: polls HM-10 every 250 ms, posts EVT_BEACON_FOUND/LOST
 *   - event_dispatcher: calls fsm_process_event() in main loop
 *   - logger_task: flushes log ring to serial
 *   - button ISR: posts EVT_BUTTON_SW2/SW3
 *
 * Architecture: bare-metal cooperative (no FreeRTOS yet).
 * The main loop is the event_dispatcher — it processes one event per
 * iteration, then polls BLE, buttons, and feedback.
 *
 * Expected output on serial monitor (115200 bps):
 *   [00000100] SYS:INFO FSM init start
 *   [00000100] SYS:INFO FSM init complete
 *   [00000350] SENSOR:INFO room=1 rssi=-69
 *   [00000350] FSM:INFO First beacon detected
 *   [00000350] FSM:INFO transition: 1
 *   [00000350] FSM:INFO LOCATOR
 *   ...
 *   (press SW3 to simulate puzzle solved)
 *   [00005000] FSM:INFO Puzzle solved!
 *   [00005000] FSM:INFO transition: 3
 *   [00005000] FSM:INFO UNLOCK_NEXT
 *   ...
 *
 * Wiring: Same as kidbytes_ble (HM-10 on P1_4/P1_5 at 9600 bps)
 *         + optional servo on P3_8 (J3 pin 11)
 *         + optional buzzer on P2_4 (J1 pin 6)
 *****************************************************************************/
#include <MCXA153.h>
#include <stdio.h>
#include <string.h>

#include "serial.h"
#include "log.h"
#include "main_fsm.h"
#include "ble_drv.h"
#include "location_engine.h"
#include "config_store.h"
#include "button_drv.h"
#include "feedback_drv.h"
#include "nvm_drv.h"
#include "admin_console.h"   /* F5: UART admin REPL (Option B) */
#include "tft_lcd.h"        /* lcd_get_touch() — refreshes lcd_touch_x/y    */
#include "lpspi_master.h"   /* touch_detected flag (set by XPT2046 IRQ)     */
#include "rssi_bar.h"       /* Layer 6 "hotter/colder" proximity HUD        */
#include "extio_drv.h"      /* external START/PLAY buttons + LEDs (item 1/3) */
#include "v_screen.h"       /* Layer 6 waypoint HOTTER/COLDER screen        */

/* -------------------------------------------------------------------------
 * Globals
 * ---------------------------------------------------------------------- */
volatile uint32_t ms = 0;

void SysTick_Handler(void) { ms++; }

/* -------------------------------------------------------------------------
 * BLE scan state
 * ---------------------------------------------------------------------- */
static ble_drv_state_t  g_ble;
static config_store_t   g_cfg;
static loc_state_t      g_loc;

/* -------------------------------------------------------------------------
 * Scan task (cooperative — called from main loop every 250 ms)
 * ---------------------------------------------------------------------- */
static uint32_t next_scan_ms = 0;

static void scan_task_poll(void)
{
    /* Time to trigger a new scan? */
    if ((int32_t)(ms - next_scan_ms) >= 0 && !ble_drv_is_scanning(&g_ble))
    {
        ble_drv_start_scan(&g_ble);
        next_scan_ms = ms + BLE_SCAN_PERIOD_MS;
    }

    /* Poll for incoming UART data */
    bool scan_done = ble_drv_poll(&g_ble);

    if (scan_done)
    {
        uint8_t room = loc_get_room(&g_loc);
        int8_t  rssi = loc_get_rssi(&g_loc);
        bool    low  = loc_is_signal_low(&g_loc);

        /* ---- Proximity feedback: use the TARGET beacon's RSSI ----
         * The bar/warmth/LED must track how close the kid is to the NEXT
         * beacon on the route, not whichever beacon happens to be strongest
         * (which may be the one they just left). Query the location engine
         * for the target room's filtered RSSI directly. */
        uint8_t target_room = fsm_get_target_room();
        int8_t  target_rssi = (target_room != 0xFF)
                              ? loc_get_room_rssi(&g_loc, target_room)
                              : (int8_t)-90;

        /* Feed the "hotter/colder" proximity HUD with the TARGET's RSSI.
         * On signal loss or no target, push RSSI_FAR (-90). */
        rssi_bar_update((!low && target_room != 0xFF && target_rssi > -100)
                        ? target_rssi : (int8_t)-90);

        if (room != LOC_NO_ROOM && !low)
        {
            /* Post beacon found event (still uses the BEST room for FSM logic) */
            fsm_event_t evt = {
                .type = EVT_BEACON_FOUND,
                .timestamp = ms,
                .data.beacon = { .room_id = room, .rssi = rssi }
            };
            fsm_post_event(&evt);

            LogWithNum(LOG_SYS_SENSOR, LOG_INFO, "room=", room + 1);
        }
        else if (low)
        {
            fsm_event_t evt = { .type = EVT_BEACON_LOST, .timestamp = ms };
            fsm_post_event(&evt);
        }

        /* ---- Hunting feedback (IDLE/LOCATOR): LCD + bar + LED ---- */
        fsm_state_t st = fsm_get_state();
        if (st == FSM_IDLE || st == FSM_LOCATOR)
        {
            int8_t r = target_rssi;    /* proximity to the TARGET, not "best" */
            bool   target_game = fsm_target_is_game();

            /* LCD: toward a GAME show room/next/puzzle; toward a WAYPOINT show
             * the big HOTTER/COLDER words (no game info). */
            if (target_game)
            {
                /* game screen is set up on LOCATOR entry by the FSM; just keep
                 * the proximity bar on top. */
            }
            else
            {
                v_screen_waypoint(r);   /* redraws only when warmth level changes */
            }
            rssi_bar_draw();

            /* LED: GREEN blinks toward a waypoint, RED toward a game; SLOW far,
             * FAST near. Solid LED at arrival is handled by the FSM (ARMED). */
            int hp = 600 + (int)(r + 55) * (600 - 120) / (90 - 55);
            if (hp < 120) hp = 120;
            if (hp > 600) hp = 600;
            extio_blink_set(target_game ? EXTIO_BLINK_ARM : EXTIO_BLINK_READY,
                            (uint16_t)hp);
        }
        else if (st == FSM_ARMED)
        {
            /* Solid LED + (game) room screen or (waypoint) warmth word already
             * on the LCD; keep the bar full/red on top. */
            rssi_bar_draw();
        }
    }
}

/* -------------------------------------------------------------------------
 * Button task (cooperative — polls button event queue)
 * ---------------------------------------------------------------------- */
static void button_task_poll(void)
{
    button_event_t btn;
    while (button_drv_get_event(&btn))
    {
        fsm_event_t evt;
        evt.timestamp = btn.timestamp;

        if (btn.id == BTN_SW2)
        {
            evt.type = EVT_BUTTON_SW2;
            Log(LOG_SYS_FSM, LOG_DEBUG, "SW2 pressed");
        }
        else
        {
            evt.type = EVT_BUTTON_SW3;
            Log(LOG_SYS_FSM, LOG_DEBUG, "SW3 pressed");
        }

        fsm_post_event(&evt);

        /* Route the press into the active puzzle (no-op outside PUZZLE) */
        fsm_puzzle_input();
    }
}

/* -------------------------------------------------------------------------
 * Touch task (cooperative — feeds the active puzzle's touch input)
 *
 * Khanh's games are touch-driven: input_detect() reads lcd_touch_x/y, which
 * are only valid after lcd_get_touch() runs. The XPT2046 IRQ sets the shared
 * touch_detected flag; here we clear it, refresh the coordinates, then route
 * the touch into the active puzzle via the same fsm_puzzle_input() contract
 * the buttons use. No-op outside PUZZLE (fsm_puzzle_input guards on state).
 * ---------------------------------------------------------------------- */
static void touch_task_poll(void)
{
    if (touch_detected)
    {
        touch_detected = false;
        lcd_get_touch();        /* refresh lcd_touch_x / lcd_touch_y (F2.1) */
        /* TEMP touch calibration aid: print raw mapped coords so we can see
         * which zone a tap lands in. Remove once touch zones are confirmed. */
        printf("TOUCH x=%d y=%d\r\n", (int)lcd_touch_x, (int)lcd_touch_y);
        fsm_puzzle_input();     /* feed the touch into Khanh's active game   */
    }
}

/* -------------------------------------------------------------------------
 * External IO task (cooperative — polls the external START/PLAY buttons)
 *
 * These are the EXTERNAL buttons on free header pins (P1_10 START, P3_30
 * PLAY), separate from the on-board SW2/SW3. extio_poll() debounces and
 * returns at most one press edge per call; we translate it into the matching
 * FSM event. The external LEDs are driven by the FSM (READY/ARMED states).
 * ---------------------------------------------------------------------- */
static void extio_task_poll(void)
{
    extio_btn_t b = extio_poll();

    if (b == EXTIO_BTN_START_SESSION)
    {
        fsm_event_t evt = { .type = EVT_START_SESSION, .timestamp = ms };
        fsm_post_event(&evt);
        Log(LOG_SYS_FSM, LOG_DEBUG, "ext START pressed");
    }
    else if (b == EXTIO_BTN_START_GAME)
    {
        fsm_event_t evt = { .type = EVT_START_GAME, .timestamp = ms };
        fsm_post_event(&evt);
        Log(LOG_SYS_FSM, LOG_DEBUG, "ext PLAY pressed");
    }
}

/* -------------------------------------------------------------------------
 * Admin task (cooperative — feeds serial RX bytes into the admin console)
 *
 * F5 Option B: the venue operator types commands in the SAME serial monitor
 * used for logs. serial_rxcnt() asks "any byte waiting?"; each byte is handed
 * to admin_console_feed(), which acts only on a full line. Non-blocking — we
 * drain at most the bytes currently in the RX FIFO, then return to the loop.
 * ---------------------------------------------------------------------- */
static void admin_task_poll(void)
{
    while (serial_rxcnt() > 0)
    {
        admin_console_feed((uint8_t)serial_getchar());
    }
}

/* -------------------------------------------------------------------------
 * Main
 * ---------------------------------------------------------------------- */
int main(void)
{
    /* System clock: 96 MHz FIRC */
    SCG0->FIRCCFG = SCG_FIRCCFG_FREQ_SEL(0b101);
    SysTick_Config(96000);  /* 1 ms tick */

    serial_init(115200);

    /* Small delay for terminal */
    uint32_t t = ms;
    while ((ms - t) < 100) {}

    printf("\r\n=== kidbytes_fsm — Layer 5 Integration ===\r\n");
    printf("Build %s %s\r\n\r\n", __DATE__, __TIME__);

    /* --- Initialize Layer 7 model --- */
    config_init(&g_cfg);
    loc_init(&g_loc, &g_cfg);

    /* --- Initialize Layer 5 admin console (F5, UART transport) --- */
    admin_console_init(&g_cfg);

    /* --- Initialize Layer 4 BLE driver --- */
    bool hm10_ok = ble_drv_init(&g_ble, &g_loc);

    /* --- Initialize FSM (inits log, nvm, lock, feedback, button) --- */
    /* Pass the config so the FSM walks the data-driven route (Option C). */
    fsm_init(&g_cfg);

    /* --- Load persisted venue config over the factory defaults (F7.3) ---
     * NVM is now initialized (inside fsm_init). If a valid config blob was
     * saved by a previous admin 'save', overlay it so the box boots with the
     * venue's room map + route instead of the sequential defaults. */
    if (nvm_drv_config_read(&g_cfg, sizeof(config_store_t)))
    {
        Log(LOG_SYS_SYS, LOG_INFO, "Loaded saved venue config from NVM");
    }
    else
    {
        Log(LOG_SYS_SYS, LOG_INFO, "No saved config — using factory defaults");
    }

    /* --- Factory reset: hold SW2 during boot to erase NVM --- */
    /* Check if P3_29 (SW2) is LOW (pressed) — read GPIO directly */
    if ((GPIO3->PDIR & (1UL << 29)) == 0)
    {
        Log(LOG_SYS_SYS, LOG_WARN, "SW2 held — erasing NVM (factory reset)");
        LogFlush();
        nvm_drv_erase_all();
        Log(LOG_SYS_SYS, LOG_INFO, "NVM erased. Release SW2 and reset.");
        LogFlush();
        /* Wait for SW2 release */
        while ((GPIO3->PDIR & (1UL << 29)) == 0) {}
        /* Software reset */
        NVIC_SystemReset();
    }

    if (hm10_ok)
    {
        Log(LOG_SYS_COMMS, LOG_INFO, "HM-10 connected");
    }
    else
    {
        Log(LOG_SYS_COMMS, LOG_WARN, "HM-10 not responding");
    }

    Log(LOG_SYS_SYS, LOG_INFO, "System ready — entering main loop");
    LogFlush();

    /* Set first scan time */
    next_scan_ms = ms + 500;

    /* =====================================================================
     * Main loop: cooperative event_dispatcher
     *
     * Architecture: scan_task → event_dispatcher → main_fsm → logger_task
     * All in one loop, no RTOS needed for this bare-metal version.
     * =================================================================== */
    while (1)
    {
        /* 1. Scan task: poll BLE, post beacon events */
        scan_task_poll();

        /* 2. Button task: poll button queue, post button events */
        button_task_poll();

        /* 2b. Touch task: feed touch into the active puzzle (Khanh's games) */
        touch_task_poll();

        /* 2c. Admin task: feed serial RX into the admin console (F5) */
        admin_task_poll();

        /* 2d. External IO: poll START/PLAY buttons (item 1/3) */
        extio_task_poll();

        /* 3. Event dispatcher: process one FSM event per iteration */
        fsm_process_event();

        /* 3b. Poll the active puzzle (posts EVT_PUZZLE_SOLVED on solve) */
        fsm_puzzle_poll();

        /* 4. Feedback: advance cue state machine */
        feedback_drv_update();

        /* 4b. External LED blink engine (slow=far, fast=near) */
        extio_blink_update();

        /* 5. Logger task: flush log ring to serial */
        LogFlush();

        /* 6. Sleep until next interrupt */
        __WFI();
    }
}
