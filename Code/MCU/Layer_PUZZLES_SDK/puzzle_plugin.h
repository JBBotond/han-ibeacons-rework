/*! ***************************************************************************
 * \brief  puzzle_plugin.h — Layer 6/7 puzzle contract (vendor-independent)
 * \file   puzzle/puzzle_plugin.h
 *
 * The uniform contract every interactive touch puzzle implements. This is the
 * swap point that lets the FSM (Layer 5) present ANY puzzle at a room without
 * knowing which one. Adding a 5th puzzle changes only a registry table —
 * never main_fsm.c.
 *
 * Khanh's 4 games already expose exactly this shape (init / input / tick),
 * so each becomes one plugin behind this contract. His active_game chaining
 * (his own FSM) is NOT used — your architecture's IDLE→LOCATOR→PUZZLE→
 * UNLOCK_NEXT→FINAL FSM stays the single controller (F2.1, F2.2, F2.3).
 *
 * Mapping to the architecture:
 *   Layer 6 VIEW   — each plugin renders to the TFT (V_SCREEN region)
 *   Layer 7 MODEL  — each plugin validates the kid's input (puzzle_engine role)
 *   Layer 5 FSM    — calls start → feed input → poll solved, UNCHANGED
 *
 * Rule (Layer 3 HAL style): pure header, only <stdint.h>/<stdbool.h>,
 * zero MCU-specific symbols, zero #include of game internals.
 *
 * FR coverage: F2.1 (start at room), F2.2 (solved → advance),
 *              F2.3 (wrong → retry, no progress change)
 *****************************************************************************/
#ifndef PUZZLE_PLUGIN_H
#define PUZZLE_PLUGIN_H

#include <stdint.h>
#include <stdbool.h>

/**
 * \brief One interactive puzzle, behind a uniform 3-function contract.
 *
 * The FSM drives every puzzle through the same lifecycle:
 *   1. start()  — render the puzzle, reset its internal state (F2.1)
 *   2. on each touch: input() — feed the touch into the puzzle
 *   3. tick()   — called every loop; returns true once SOLVED (F2.2)
 *
 * A wrong answer is handled INSIDE the puzzle (re-prompt). tick() simply
 * keeps returning false until the kid gets it right (F2.3 — no progress
 * change on wrong input; the FSM never advances until tick() == true).
 */
typedef struct
{
    const char *name;            /**< puzzle label, e.g. "RPS", "QUIZ" */
    void (*start)(void);         /**< render + reset (F2.1)            */
    void (*input)(void);         /**< feed one touch event             */
    bool (*tick)(void);          /**< true once solved (F2.2)          */
} puzzle_plugin_t;

#endif /* PUZZLE_PLUGIN_H */
