/*! ***************************************************************************
 * \brief  puzzle_engine — Layer 7 Model (host-testable)
 * \file   puzzle_engine.h
 *
 * Validates puzzle answers and tracks collected digits for the final code.
 *
 * Design:
 *   - Each room (0..4) has a puzzle. The answer is a short string stored
 *     in config_store (e.g. "3", "7", "TICTAC", etc.).
 *   - When the kid solves a room's puzzle, they earn a "digit" (the first
 *     character of that room's answer). These 5 digits form the final code.
 *   - The FINAL room's puzzle (F2.4): kid must enter the 5-digit code
 *     composed of digits collected from rooms 0..4 in route order.
 *
 * Dependencies: config_store.h, <stdint.h>, <stdbool.h>, <string.h>.
 *               No MCU calls.
 * Depends on: config_store (puzzle answers per room).
 * Depended on by: main_fsm (Layer 5), route_manager (advance on solve).
 *
 * FR coverage:
 *   F2.1 — start puzzle when target room entered
 *   F2.2 — reveal next room after puzzle solved
 *   F2.3 — incorrect input → retry, no progress change
 *   F2.4 — final room = 5-digit collected code
 *****************************************************************************/
#ifndef PUZZLE_ENGINE_H
#define PUZZLE_ENGINE_H

#include <stdint.h>
#include <stdbool.h>
#include "config_store.h"

/* -------------------------------------------------------------------------
 * Constants
 * ---------------------------------------------------------------------- */

/** Maximum collected digits (one per room). */
#define PUZZLE_MAX_DIGITS  CONFIG_MAX_ROOMS

/* -------------------------------------------------------------------------
 * Types
 * ---------------------------------------------------------------------- */

/** Puzzle engine state. */
typedef struct
{
    const config_store_t *cfg;
    uint8_t  active_room;                        /**< room index of current puzzle, or 0xFF */
    char     collected[PUZZLE_MAX_DIGITS + 1];   /**< digits earned so far (NUL-term) */
    uint8_t  collected_count;                    /**< how many digits collected (0..5) */
    bool     solved;                             /**< was the current puzzle solved?   */
} puzzle_state_t;

/* -------------------------------------------------------------------------
 * API
 * ---------------------------------------------------------------------- */

/**
 * \brief Initialize the puzzle engine.
 * \param state  Caller-owned state struct.
 * \param cfg    Pointer to initialized config_store (must outlive state).
 */
void puzzle_init(puzzle_state_t *state, const config_store_t *cfg);

/**
 * \brief Start the puzzle for a given room (F2.1).
 *
 * Called by the FSM when the kid enters the target room.
 * Resets the "solved" flag for this room.
 *
 * \param state       Engine state.
 * \param room_index  Which room's puzzle to activate (0..4).
 */
void puzzle_start(puzzle_state_t *state, uint8_t room_index);

/**
 * \brief Check the kid's answer against the configured answer (F2.2, F2.3).
 *
 * \param state  Engine state.
 * \param input  The kid's answer string (NUL-terminated).
 * \return true if correct (puzzle solved → collect digit, mark solved).
 *         false if incorrect (F2.3: no progress change, retry allowed).
 */
bool puzzle_check_answer(puzzle_state_t *state, const char *input);

/**
 * \brief Check if the current puzzle has been solved.
 */
bool puzzle_is_solved(const puzzle_state_t *state);

/**
 * \brief Get the digit collected from a specific room.
 * \param state       Engine state.
 * \param room_index  Which room (0..4).
 * \return The digit character, or '\0' if not yet collected.
 */
char puzzle_get_collected_digit(const puzzle_state_t *state, uint8_t room_index);

/**
 * \brief Get the full collected code string so far.
 * \return Pointer to NUL-terminated string of collected digits (length = collected_count).
 */
const char *puzzle_get_collected_code(const puzzle_state_t *state);

/**
 * \brief Build the expected final code from all rooms in route order (F2.4).
 *
 * The final code = first char of each room's answer, in route order.
 * This is what the kid must enter at the last room.
 *
 * \param state   Engine state.
 * \param out     Output buffer (must be at least PUZZLE_MAX_DIGITS + 1 bytes).
 * \return Number of characters written (= room_count), or 0 on error.
 */
uint8_t puzzle_build_final_code(const puzzle_state_t *state, char *out);

/**
 * \brief Reset all collected digits (e.g. new session).
 */
void puzzle_reset(puzzle_state_t *state);

#endif /* PUZZLE_ENGINE_H */
