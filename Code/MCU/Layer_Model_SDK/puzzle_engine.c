/*! ***************************************************************************
 * \brief  puzzle_engine — Layer 7 Model implementation
 * \file   puzzle_engine.c
 *
 * Pure C, no MCU dependencies. Compile with gcc on any host.
 *****************************************************************************/
#include "puzzle_engine.h"
#include <stddef.h>
#include <string.h>

/* -------------------------------------------------------------------------
 * Init / Reset
 * ---------------------------------------------------------------------- */

void puzzle_init(puzzle_state_t *state, const config_store_t *cfg)
{
    if (state == NULL || cfg == NULL) return;

    state->cfg             = cfg;
    state->active_room     = 0xFF;
    state->collected_count = 0;
    state->solved          = false;
    memset(state->collected, '\0', sizeof(state->collected));
}

void puzzle_reset(puzzle_state_t *state)
{
    if (state == NULL) return;

    state->active_room     = 0xFF;
    state->collected_count = 0;
    state->solved          = false;
    memset(state->collected, '\0', sizeof(state->collected));
}

/* -------------------------------------------------------------------------
 * Puzzle lifecycle
 * ---------------------------------------------------------------------- */

void puzzle_start(puzzle_state_t *state, uint8_t room_index)
{
    if (state == NULL || state->cfg == NULL) return;
    if (room_index >= config_get_room_count(state->cfg)) return;

    state->active_room = room_index;
    state->solved      = false;
}

bool puzzle_check_answer(puzzle_state_t *state, const char *input)
{
    if (state == NULL || state->cfg == NULL || input == NULL) return false;
    if (state->active_room == 0xFF) return false;  /* no puzzle active */

    const config_room_t *room = config_get_room(state->cfg, state->active_room);
    if (room == NULL) return false;

    /* Compare kid's input against configured answer (case-sensitive) */
    if (strcmp(input, room->answer) != 0)
    {
        /* F2.3: incorrect → no progress change, retry allowed */
        return false;
    }

    /* Correct answer — collect the digit (first char of answer) */
    if (state->collected_count < PUZZLE_MAX_DIGITS)
    {
        state->collected[state->collected_count] = room->answer[0];
        state->collected_count++;
        state->collected[state->collected_count] = '\0';
    }

    state->solved = true;
    return true;
}

/* -------------------------------------------------------------------------
 * Getters
 * ---------------------------------------------------------------------- */

bool puzzle_is_solved(const puzzle_state_t *state)
{
    if (state == NULL) return false;
    return state->solved;
}

char puzzle_get_collected_digit(const puzzle_state_t *state, uint8_t room_index)
{
    if (state == NULL) return '\0';
    if (room_index >= state->collected_count) return '\0';
    return state->collected[room_index];
}

const char *puzzle_get_collected_code(const puzzle_state_t *state)
{
    if (state == NULL) return "";
    return state->collected;
}

uint8_t puzzle_build_final_code(const puzzle_state_t *state, char *out)
{
    if (state == NULL || state->cfg == NULL || out == NULL) return 0;

    uint8_t room_count = config_get_room_count(state->cfg);
    const uint8_t *route = config_get_route(state->cfg);
    if (route == NULL) return 0;

    for (uint8_t i = 0; i < room_count; i++)
    {
        const config_room_t *room = config_get_room(state->cfg, route[i]);
        if (room == NULL || room->answer[0] == '\0')
        {
            out[i] = '?';
        }
        else
        {
            out[i] = room->answer[0];
        }
    }
    out[room_count] = '\0';

    return room_count;
}
