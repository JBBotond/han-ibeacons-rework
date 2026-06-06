/*! ***************************************************************************
 * \brief  log_model — Layer 7 Model implementation
 * \file   log_model.c
 *
 * Pure C, no MCU dependencies. Compile with gcc on any host.
 *
 * Ring buffer: when full, head wraps around and overwrites oldest entries.
 * Read access is always oldest-first (index 0 = oldest).
 *****************************************************************************/
#include "log_model.h"
#include <stddef.h>
#include <string.h>

/* -------------------------------------------------------------------------
 * Init / Clear
 * ---------------------------------------------------------------------- */

void logm_init(log_state_t *state)
{
    if (state == NULL) return;

    memset(state->entries, 0, sizeof(state->entries));
    state->head  = 0;
    state->count = 0;
}

void logm_clear(log_state_t *state)
{
    if (state == NULL) return;
    logm_init(state);
}

/* -------------------------------------------------------------------------
 * Append
 * ---------------------------------------------------------------------- */

void logm_append(log_state_t *state, uint8_t room_id,
                 uint32_t timestamp_ms, uint8_t outcome)
{
    if (state == NULL) return;

    log_entry_t *entry = &state->entries[state->head];
    entry->timestamp_ms = timestamp_ms;
    entry->room_id      = room_id;
    entry->outcome      = outcome;
    entry->_reserved[0] = 0;
    entry->_reserved[1] = 0;

    /* Advance head (ring wrap) */
    state->head = (state->head + 1) % LOG_MAX_ENTRIES;

    /* Track count up to max */
    if (state->count < LOG_MAX_ENTRIES)
    {
        state->count++;
    }
}

/* -------------------------------------------------------------------------
 * Getters
 * ---------------------------------------------------------------------- */

uint16_t logm_get_count(const log_state_t *state)
{
    if (state == NULL) return 0;
    return state->count;
}

const log_entry_t *logm_get_entry(const log_state_t *state, uint16_t index)
{
    if (state == NULL) return NULL;
    if (index >= state->count) return NULL;

    /* Index 0 = oldest entry.
     * If buffer not full: oldest is at position 0.
     * If buffer full: oldest is at position head (it's the next to be overwritten).
     */
    uint16_t start;
    if (state->count < LOG_MAX_ENTRIES)
    {
        start = 0;
    }
    else
    {
        start = state->head;  /* head points to the oldest when full */
    }

    uint16_t actual = (start + index) % LOG_MAX_ENTRIES;
    return &state->entries[actual];
}

const log_entry_t *logm_get_latest(const log_state_t *state)
{
    if (state == NULL || state->count == 0) return NULL;

    /* Latest is one position before head */
    uint16_t latest_idx = (state->head == 0) ? (LOG_MAX_ENTRIES - 1) : (state->head - 1);
    return &state->entries[latest_idx];
}

uint32_t logm_get_session_duration_ms(const log_state_t *state)
{
    if (state == NULL || state->count < 2) return 0;

    const log_entry_t *oldest = logm_get_entry(state, 0);
    const log_entry_t *newest = logm_get_latest(state);

    if (oldest == NULL || newest == NULL) return 0;

    return newest->timestamp_ms - oldest->timestamp_ms;
}

uint32_t logm_get_room_time_ms(const log_state_t *state, uint8_t room_id)
{
    if (state == NULL || state->count == 0) return 0;

    uint32_t enter_time = 0;
    bool     found_enter = false;

    /* Scan from oldest to newest looking for ENTER then SOLVED for this room */
    for (uint16_t i = 0; i < state->count; i++)
    {
        const log_entry_t *e = logm_get_entry(state, i);
        if (e == NULL) continue;

        if (e->room_id != room_id) continue;

        if (e->outcome == LOG_OUTCOME_ENTER && !found_enter)
        {
            enter_time = e->timestamp_ms;
            found_enter = true;
        }
        else if (e->outcome == LOG_OUTCOME_SOLVED && found_enter)
        {
            return e->timestamp_ms - enter_time;
        }
    }

    return 0;  /* ENTER or SOLVED not found for this room */
}
