/*! ***************************************************************************
 * \brief  main_fsm.h — Layer 5 Functional Design (Active Object)
 * \file   main_fsm.h
 *
 * Single controller that owns every state change. Tasks hand events to it
 * through a priority queue. No shared state, no locks needed.
 *
 * States: IDLE → LOCATOR → PUZZLE → UNLOCK_NEXT → FINAL
 * Super-states: ADMIN, FAULT
 *
 * Dependencies: All Layer 4 drivers, Layer 7 model, Layer 6 view, Layer 11 log
 * Depended on by: Nothing — this IS the top-level controller
 *
 * FR coverage: F1–F8 integration
 *****************************************************************************/
#ifndef MAIN_FSM_H
#define MAIN_FSM_H

#include <stdint.h>
#include <stdbool.h>
#include "config_store.h"   /* route + room map (Layer 7 model) */

/* -------------------------------------------------------------------------
 * FSM States
 * ---------------------------------------------------------------------- */
typedef enum
{
    FSM_IDLE         = 0,   /**< Power-on, waiting for first beacon     */
    FSM_LOCATOR      = 1,   /**< Scanning for target room               */
    FSM_PUZZLE       = 2,   /**< Puzzle active, waiting for input        */
    FSM_UNLOCK_NEXT  = 3,   /**< Puzzle solved, revealing next room      */
    FSM_FINAL        = 4,   /**< All rooms done, lock opening            */
    FSM_ADMIN        = 5,   /**< Admin mode via USB-CDC                  */
    FSM_FAULT        = 6,   /**< Error state (BLE_LOST, NVM_FAIL, etc.)  */
    FSM_READY        = 7,   /**< Ready: green LED, press START (item 1)  */
    FSM_ARMED        = 8,   /**< Near beacon: blue LED, press play (item 3) */
    FSM_STATE_COUNT  = 9
} fsm_state_t;

/* -------------------------------------------------------------------------
 * Event Types (what scan_task / button_drv / usb produce)
 * ---------------------------------------------------------------------- */
typedef enum
{
    EVT_NONE          = 0,
    EVT_BEACON_FOUND  = 1,   /**< location_engine resolved a room        */
    EVT_BEACON_LOST   = 2,   /**< signal_low after 3 missed scans        */
    EVT_BUTTON_SW2    = 3,   /**< SW2 pressed (puzzle input A)            */
    EVT_BUTTON_SW3    = 4,   /**< SW3 pressed (puzzle input B)            */
    EVT_PUZZLE_SOLVED = 5,   /**< puzzle_engine reports correct answer    */
    EVT_PUZZLE_FAIL   = 6,   /**< puzzle_engine reports wrong answer      */
    EVT_ADMIN_ENTER   = 7,   /**< USB-CDC shared secret received          */
    EVT_ADMIN_EXIT    = 8,   /**< Admin session ended                     */
    EVT_ADMIN_UNLOCK  = 9,   /**< Admin requests manual lock open         */
    EVT_FAULT         = 10,  /**< Unrecoverable error                     */
    EVT_TICK          = 11,  /**< Periodic 250 ms tick from scan_task     */
    EVT_START_SESSION = 12,  /**< External START btn (P1_10): begin session (item 1) */
    EVT_START_GAME    = 13,  /**< External PLAY  btn (P3_30): start game    (item 3) */
    EVT_COUNT         = 14
} fsm_event_type_t;

/* -------------------------------------------------------------------------
 * Event structure (passed through the dispatcher queue)
 * ---------------------------------------------------------------------- */
typedef struct
{
    fsm_event_type_t type;
    uint32_t         timestamp;  /**< ms tick when event was created      */
    union
    {
        struct { uint8_t room_id; int8_t rssi; } beacon;
        struct { uint8_t button_id; } button;
        struct { uint8_t digit; } puzzle_input;
        struct { uint8_t fault_code; } fault;
    } data;
} fsm_event_t;

/* -------------------------------------------------------------------------
 * Event Queue (16 slots as per architecture)
 * ---------------------------------------------------------------------- */
#define FSM_QUEUE_SIZE  16

/* -------------------------------------------------------------------------
 * Proximity arm threshold (item 3: "kid within ~2 m of beacon")
 *
 * Filtered RSSI at ~2 m is roughly -70 dBm for the HM-10/iBeacon at default
 * TX power. When the target room's filtered RSSI rises to (>=) this value the
 * FSM leaves LOCATOR for ARMED (blue LED, press the play button). TUNE THIS
 * on the bench: walk to 2 m, read the SENSOR rssi log, set the number you see.
 * Less negative = must be closer; more negative = arms from farther away.
 * ---------------------------------------------------------------------- */
#define FSM_ARM_RSSI_DBM   (-70)

/* Waypoint arm threshold — kid must get CLOSER (~1 m) to a transition beacon
 * before its green LED goes solid and the button advances the route. Closer
 * than the game threshold so the trail feels like "step right up to it". */
#define FSM_ARM_RSSI_WP_DBM   (-60)

/* Disarm hysteresis (dB): once armed, only disarm if RSSI drops this much
 * below the arm threshold, so hovering at the edge doesn't flicker the LED. */
#define FSM_ARM_HYSTERESIS    (6)

/* -------------------------------------------------------------------------
 * API
 * ---------------------------------------------------------------------- */

/**
 * \brief Initialize the FSM and all subsystems.
 *
 * Calls init for: log, ble_drv, display_drv, button_drv, lock_drv,
 * feedback_drv, nvm_drv, config_store, location_engine, puzzle_engine,
 * route_manager.
 *
 * Starts in FSM_IDLE state.
 *
 * \param cfg  Pointer to an initialized config_store (route + room map).
 *             Must outlive the FSM. If NULL, the FSM falls back to a
 *             sequential 0,1,2,... route.
 */
void fsm_init(const config_store_t *cfg);

/**
 * \brief Post an event to the FSM queue.
 * \param evt  Pointer to the event to enqueue.
 * \return true if enqueued, false if queue full (event dropped).
 *
 * Called from ISR context (button_drv) or task context (scan_task).
 * Thread-safe via disable_irq around ring operations.
 */
bool fsm_post_event(const fsm_event_t *evt);

/**
 * \brief Process one event from the queue (called by event_dispatcher).
 * \return true if an event was processed, false if queue empty.
 *
 * This is the heart of the FSM — the state transition logic.
 */
bool fsm_process_event(void);

/**
 * \brief Poll the active puzzle while in PUZZLE state (call from main loop).
 *
 * Asks the active game "solved yet?" via its plugin tick(). On solve it
 * posts EVT_PUZZLE_SOLVED. No-op outside PUZZLE state. (F2.2)
 */
void fsm_puzzle_poll(void);

/**
 * \brief Route a touch/button event into the active puzzle.
 *
 * Called by the dispatcher when a touch/button arrives during PUZZLE.
 * No-op outside PUZZLE state.
 */
void fsm_puzzle_input(void);

/**
 * \brief Get the current FSM state.
 */
fsm_state_t fsm_get_state(void);

/**
 * \brief Get the current FSM state as a string (for logging).
 */
const char *fsm_state_name(fsm_state_t state);

/**
 * \brief Get the current route progress (rooms completed, 0–5).
 */
uint8_t fsm_get_progress(void);

/**
 * \brief Is the current route step a GAME beacon (vs a WAYPOINT)?
 *
 * Drives the external LED color: red blink toward a game, green blink toward
 * a waypoint. Returns true (game) when no config is present.
 */
bool fsm_target_is_game(void);

/**
 * \brief Get the current target room index (0-based beacon slot).
 *
 * The scan task uses this to read the target's RSSI from the location engine
 * so proximity feedback (bar, warmth word, LED blink rate) tracks the correct
 * beacon — not just whichever beacon happens to be strongest.
 *
 * \return Room index (0..N), or 0xFF if no target is set (FSM_READY/FINAL).
 */
uint8_t fsm_get_target_room(void);

#endif /* MAIN_FSM_H */
