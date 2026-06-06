/*! ***************************************************************************
 * \brief  log_model — Layer 7 Model (host-testable)
 * \file   log_model.h
 *
 * Session log: stores timestamped events (room transitions, outcomes).
 * Ring buffer design — oldest entries are overwritten when full.
 * The PC companion (V_PC) downloads this via USB-CDC for F6.3.
 *
 * Dependencies: <stdint.h>, <stdbool.h>. No MCU calls.
 * Depends on: nothing (sink only — other modules push events into it).
 * Depended on by: usb_drv (Layer 4, for download), V_PC (Layer 6).
 *
 * FR coverage:
 *   F7.2 — persist {room_id, timestamp, outcome} on FSM transitions
 *   F7.3 — resume in IDLE with last session readable after power restore
 *   F6.3 — PC app downloads ordered route, time-per-room, total session time
 *   F8   — cross-cutting log (this is the structured data log, not printf)
 *****************************************************************************/
#ifndef LOG_MODEL_H
#define LOG_MODEL_H

#include <stdint.h>
#include <stdbool.h>

/* -------------------------------------------------------------------------
 * Constants
 * ---------------------------------------------------------------------- */

/** Maximum log entries in the ring buffer.
 *  32 entries × 8 bytes = 256 bytes — fits comfortably in SRAM.
 *  A full 5-room tour produces ~12 entries (enter + solve per room + final).
 */
#define LOG_MAX_ENTRIES  32

/* -------------------------------------------------------------------------
 * Types
 * ---------------------------------------------------------------------- */

/** Outcome codes for log entries. */
typedef enum
{
    LOG_OUTCOME_ENTER     = 0x01,  /**< Kid entered a room                */
    LOG_OUTCOME_SOLVED    = 0x02,  /**< Puzzle solved correctly            */
    LOG_OUTCOME_FAILED    = 0x03,  /**< Puzzle attempt failed              */
    LOG_OUTCOME_FINAL     = 0x04,  /**< Route complete, lock opened        */
    LOG_OUTCOME_ADMIN     = 0x05,  /**< Admin session started              */
    LOG_OUTCOME_RESET     = 0x06,  /**< Session reset (admin or power)     */
    LOG_OUTCOME_SIGNAL_LOW = 0x07, /**< Signal lost for 3+ scans           */
} log_outcome_t;

/** One log entry — 8 bytes, aligned for NVM write. */
typedef struct
{
    uint32_t timestamp_ms;  /**< ms since boot (from SysTick)            */
    uint8_t  room_id;       /**< room index (0..4) or 0xFF for system    */
    uint8_t  outcome;       /**< log_outcome_t value                     */
    uint8_t  _reserved[2];  /**< padding to 8 bytes for NVM alignment    */
} log_entry_t;

/** Log model state (ring buffer). */
typedef struct
{
    log_entry_t entries[LOG_MAX_ENTRIES];
    uint16_t    head;       /**< next write position                     */
    uint16_t    count;      /**< total entries stored (max = LOG_MAX)    */
} log_state_t;

/* -------------------------------------------------------------------------
 * API
 * ---------------------------------------------------------------------- */

/**
 * \brief Initialize the log model (empty ring buffer).
 */
void logm_init(log_state_t *state);

/**
 * \brief Append a log entry (F7.2).
 *
 * If the buffer is full, the oldest entry is overwritten (ring behavior).
 *
 * \param state         Log state.
 * \param room_id       Room index (0..4) or 0xFF for system events.
 * \param timestamp_ms  Milliseconds since boot.
 * \param outcome       What happened (log_outcome_t).
 */
void logm_append(log_state_t *state, uint8_t room_id,
                 uint32_t timestamp_ms, uint8_t outcome);

/**
 * \brief Get total number of entries stored.
 * \return 0..LOG_MAX_ENTRIES.
 */
uint16_t logm_get_count(const log_state_t *state);

/**
 * \brief Get a log entry by index (0 = oldest).
 * \return Pointer to entry, or NULL if index >= count.
 */
const log_entry_t *logm_get_entry(const log_state_t *state, uint16_t index);

/**
 * \brief Get the most recent entry.
 * \return Pointer to newest entry, or NULL if empty.
 */
const log_entry_t *logm_get_latest(const log_state_t *state);

/**
 * \brief Calculate total session time (newest timestamp - oldest timestamp).
 * \return Duration in ms, or 0 if fewer than 2 entries.
 */
uint32_t logm_get_session_duration_ms(const log_state_t *state);

/**
 * \brief Calculate time spent in a specific room (F6.3: time-per-room).
 *
 * Finds the ENTER and SOLVED events for the given room and returns
 * the difference. Returns 0 if not found.
 *
 * \param state    Log state.
 * \param room_id  Room index (0..4).
 * \return Time in ms between ENTER and SOLVED for that room, or 0.
 */
uint32_t logm_get_room_time_ms(const log_state_t *state, uint8_t room_id);

/**
 * \brief Clear all log entries.
 */
void logm_clear(log_state_t *state);

#endif /* LOG_MODEL_H */
