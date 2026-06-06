/*! ***************************************************************************
 * \brief  route_manager — Layer 7 Model (host-testable)
 * \file   route_manager.h
 *
 * Tracks the kid's progress through the 5-room sequence.
 * Consumes config_store (route order) and location_engine (current room).
 *
 * Dependencies: config_store.h, <stdint.h>, <stdbool.h>. No MCU calls.
 * Depended on by: main_fsm (Layer 5), V_ICONS (k/5 display).
 *
 * FR coverage: F2.1 (start puzzle when target room entered),
 *              F2.2 (reveal next room after puzzle solved),
 *              F3.4 (Room k/5 indicator).
 *****************************************************************************/
#ifndef ROUTE_MANAGER_H
#define ROUTE_MANAGER_H

#include <stdint.h>
#include <stdbool.h>
#include "config_store.h"

/* -------------------------------------------------------------------------
 * Types
 * ---------------------------------------------------------------------- */

/** Route state — opaque to callers, but defined here for stack allocation. */
typedef struct
{
    const config_store_t *cfg;   /**< pointer to config (read-only)       */
    uint8_t k;                   /**< current progress: 0 = not started,
                                      1..5 = rooms completed              */
} route_state_t;

/* -------------------------------------------------------------------------
 * API
 * ---------------------------------------------------------------------- */

/**
 * \brief Initialize the route manager with a config store.
 * \param state  Caller-owned state struct.
 * \param cfg    Pointer to initialized config_store (must outlive state).
 *
 * After init: progress = 0, target = route[0].
 */
void route_init(route_state_t *state, const config_store_t *cfg);

/**
 * \brief Get the room INDEX that the kid should go to next.
 * \return Room index (0..4) from config route[], or 0xFF if route complete.
 *
 * This is the room the FSM compares against location_engine output.
 * WHEN the user enters this room → start puzzle (F2.1).
 */
uint8_t route_get_current_target(const route_state_t *state);

/**
 * \brief Get current progress (how many rooms completed).
 * \return 0..5. Used by V_ICONS for "k/5" display (F3.4).
 */
uint8_t route_get_progress(const route_state_t *state);

/**
 * \brief Advance to the next room after puzzle solved (F2.2).
 * \return true if the route is now COMPLETE (k == room_count → FINAL state).
 *         false if there are more rooms to visit.
 *
 * Call this ONLY after puzzle_engine confirms correct answer.
 */
bool route_advance(route_state_t *state);

/**
 * \brief Check if the full route is complete.
 * \return true if k == room_count (all rooms visited + puzzles solved).
 *
 * When true → FSM transitions to FINAL → lock opens (F4.1).
 */
bool route_is_complete(const route_state_t *state);

/**
 * \brief Reset progress to 0 (e.g. admin restart or new session).
 */
void route_reset(route_state_t *state);

#endif /* ROUTE_MANAGER_H */
