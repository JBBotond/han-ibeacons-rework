/*! ***************************************************************************
 * \brief  route_manager — Layer 7 Model implementation
 * \file   route_manager.c
 *
 * Pure C, no MCU dependencies. Compile with gcc on any host.
 *****************************************************************************/
#include "route_manager.h"
#include <stddef.h>

/* -------------------------------------------------------------------------
 * Init / Reset
 * ---------------------------------------------------------------------- */

void route_init(route_state_t *state, const config_store_t *cfg)
{
    if (state == NULL || cfg == NULL) return;

    state->cfg = cfg;
    state->k   = 0;
}

void route_reset(route_state_t *state)
{
    if (state == NULL) return;
    state->k = 0;
}

/* -------------------------------------------------------------------------
 * Getters
 * ---------------------------------------------------------------------- */

uint8_t route_get_current_target(const route_state_t *state)
{
    if (state == NULL || state->cfg == NULL) return 0xFF;

    uint8_t room_count = config_get_room_count(state->cfg);

    /* Route complete — no more targets */
    if (state->k >= room_count) return 0xFF;

    /* route[k] gives the room index for the current step */
    const uint8_t *route = config_get_route(state->cfg);
    if (route == NULL) return 0xFF;

    return route[state->k];
}

uint8_t route_get_progress(const route_state_t *state)
{
    if (state == NULL) return 0;
    return state->k;
}

bool route_is_complete(const route_state_t *state)
{
    if (state == NULL || state->cfg == NULL) return false;

    return (state->k >= config_get_room_count(state->cfg));
}

/* -------------------------------------------------------------------------
 * State transitions
 * ---------------------------------------------------------------------- */

bool route_advance(route_state_t *state)
{
    if (state == NULL || state->cfg == NULL) return false;

    uint8_t room_count = config_get_room_count(state->cfg);

    /* Already complete — don't advance past the end */
    if (state->k >= room_count) return true;

    state->k++;

    return (state->k >= room_count);
}
