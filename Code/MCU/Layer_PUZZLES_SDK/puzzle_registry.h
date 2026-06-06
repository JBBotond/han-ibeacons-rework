/*! ***************************************************************************
 * \brief  puzzle_registry.h — maps room index → puzzle plugin
 * \file   puzzle/puzzle_registry.h
 *
 * The ONLY place that knows which puzzle lives in which room. The FSM asks
 * "give me the puzzle for room k" and drives it through the plugin contract.
 * Re-ordering puzzles or adding a 5th changes only this table — never the FSM.
 *
 * Room → puzzle assignment (5 rooms, Khanh's 4 games + final code):
 *   Room 0 → RPS    (Rock-Paper-Scissors)
 *   Room 1 → QUIZ   (mental math x3)
 *   Room 2 → MOLE   (whack-a-mole)
 *   Room 3 → SIMON  (Simon Says)
 *   Room 4 → QUIZ   (second quiz round, before FINAL)
 *
 * Dependencies: puzzle_plugin.h
 * Depended on by: main_fsm (Layer 5) PUZZLE state
 *
 * FR coverage: F2.1, F2.2, F2.3
 *****************************************************************************/
#ifndef PUZZLE_REGISTRY_H
#define PUZZLE_REGISTRY_H

#include "puzzle_plugin.h"
#include <stdint.h>

/**
 * \brief Get the puzzle plugin assigned to a room.
 * \param room_index  0..4 (route order).
 * \return Pointer to the plugin, or NULL if room_index out of range.
 */
const puzzle_plugin_t *puzzle_registry_get(uint8_t room_index);

/**
 * \brief Number of registered puzzles.
 */
uint8_t puzzle_registry_count(void);

#endif /* PUZZLE_REGISTRY_H */
