/*! ***************************************************************************
 * \brief  location_engine — Layer 7 Model implementation
 * \file   location_engine.c
 *
 * Pure C, no MCU dependencies. Compile with gcc on any host.
 *
 * RSSI filter: IIR first-order, α = 0.5 (integer arithmetic).
 *   filtered[n] = (raw[n] + filtered[n-1]) / 2
 *
 * Room decision (per scan cycle):
 *   Among all config rooms seen this scan, pick the one with the
 *   highest (least negative) filtered RSSI. If none seen, retain
 *   last room and increment miss counter.
 *****************************************************************************/
#include "location_engine.h"
#include <stddef.h>

/* -------------------------------------------------------------------------
 * Init
 * ---------------------------------------------------------------------- */

void loc_init(loc_state_t *state, const config_store_t *cfg)
{
    if (state == NULL || cfg == NULL) return;

    state->cfg          = cfg; //"I borrow this, I don't own it, I only read it, and it must outlive me."
    state->current_room = LOC_NO_ROOM;  // 0xFF = "I don't know where I am yet"
    state->current_rssi = 0; //  no signal measured yet
    state->miss_count   = 0; //   haven't missed any scans yet
    state->signal_low   = false; //  no signal-loss alarm yet

    for (uint8_t i = 0; i < CONFIG_MAX_ROOMS; i++)
    {
        state->rooms[i].rssi_filtered  = -100;  /* start pessimistic */
        state->rooms[i].seen_this_scan = false;
    }
}

/* -------------------------------------------------------------------------
 * Feed one beacon observation
 * ---------------------------------------------------------------------- */

void loc_feed_beacon(loc_state_t *state, uint16_t major, uint16_t minor, int8_t rssi)
{
    if (state == NULL || state->cfg == NULL) return;

    uint8_t room_count = config_get_room_count(state->cfg);

    /* Match Major/Minor against configured rooms */
    for (uint8_t i = 0; i < room_count; i++)
    {
        const config_room_t *room = config_get_room(state->cfg, i);
        if (room == NULL) continue;

        if (room->major == major && room->minor == minor)
        {
            /* Found a matching room — update RSSI filter */
            state->rooms[i].seen_this_scan = true;

            /* IIR filter: filtered = (raw + prev) / 2  (α = 0.5) */
            int16_t prev = state->rooms[i].rssi_filtered;
            state->rooms[i].rssi_filtered = (int16_t)((rssi + prev) / 2);

            break;  /* Major/Minor is unique per room */
        }
    }
}

/* -------------------------------------------------------------------------
 * End of scan — make room decision
 * ---------------------------------------------------------------------- */

void loc_end_scan(loc_state_t *state)
{
    if (state == NULL || state->cfg == NULL) return;

    uint8_t room_count = config_get_room_count(state->cfg);
    uint8_t best_room  = LOC_NO_ROOM;
    int16_t best_rssi  = -127;  /* worst possible */

    /* Find the room with highest filtered RSSI among those seen (F1.4) */
    for (uint8_t i = 0; i < room_count; i++)
    {
        if (state->rooms[i].seen_this_scan)
        {
            if (state->rooms[i].rssi_filtered > best_rssi)
            {
                best_rssi = state->rooms[i].rssi_filtered;
                best_room = i;
            }
        }
    }

    if (best_room != LOC_NO_ROOM)
    {
        /* At least one room beacon was seen this scan */
        state->current_room = best_room;
        state->current_rssi = (int8_t)best_rssi;
        state->miss_count   = 0;
        state->signal_low   = false;
    }
    else
    {
        /* No matching beacon seen this scan (F1.3) */
        state->miss_count++;

        if (state->miss_count >= LOC_MISS_THRESHOLD)
        {
            state->signal_low = true;
        }
        /* current_room is RETAINED (F1.3: "retain last valid room") */
    }

    /* Reset per-scan flags for next cycle */
    for (uint8_t i = 0; i < room_count; i++)
    {
        state->rooms[i].seen_this_scan = false;
    }
}

/* -------------------------------------------------------------------------
 * Getters
 * ---------------------------------------------------------------------- */

uint8_t loc_get_room(const loc_state_t *state)
{
    if (state == NULL) return LOC_NO_ROOM;
    return state->current_room;
}

int8_t loc_get_rssi(const loc_state_t *state)
{
    if (state == NULL) return 0;
    return state->current_rssi;
}

bool loc_is_signal_low(const loc_state_t *state)
{
    if (state == NULL) return false;
    return state->signal_low;
}

bool loc_is_rssi_weak(const loc_state_t *state)
{
    if (state == NULL) return false;
    if (state->current_room == LOC_NO_ROOM) return false;
    return (state->current_rssi < LOC_RSSI_LOW_THRESHOLD);
}

int8_t loc_get_room_rssi(const loc_state_t *state, uint8_t room_index)
{
    if (state == NULL || state->cfg == NULL) return -100;
    if (room_index >= config_get_room_count(state->cfg)) return -100;
    return (int8_t)state->rooms[room_index].rssi_filtered;
}
