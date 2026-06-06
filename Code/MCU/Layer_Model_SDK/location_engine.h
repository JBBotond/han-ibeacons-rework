/*! ***************************************************************************
 * \brief  location_engine — Layer 7 Model (host-testable)
 * \file   location_engine.h
 *
 * Converts raw iBeacon scan data (Major/Minor/RSSI) into a room decision.
 * Implements RSSI filtering (IIR α=0.5) and signal-loss detection.
 *
 * Dependencies: config_store.h, <stdint.h>, <stdbool.h>. No MCU calls.
 * Depends on: config_store (room map for Major/Minor → room index lookup).
 * Depended on by: main_fsm (Layer 5), route_manager (room comparison).
 *
 * FR coverage:
 *   F1.1 — scan cycle processing (called per scan)
 *   F1.2 — Major/Minor → room resolution
 *   F1.3 — 3 missed scans → SIGNAL_LOW
 *   F1.4 — multi-beacon: select highest filtered RSSI
 *****************************************************************************/
#ifndef LOCATION_ENGINE_H
#define LOCATION_ENGINE_H

#include <stdint.h>
#include <stdbool.h>
#include "config_store.h"

/* -------------------------------------------------------------------------
 * Constants
 * ---------------------------------------------------------------------- */

/** Number of consecutive empty scans before SIGNAL_LOW (F1.3: ≥750 ms) */
#define LOC_MISS_THRESHOLD  3

/** RSSI threshold for low-signal warning icon (F3.5) */
#define LOC_RSSI_LOW_THRESHOLD  (-85)

/** No valid room detected */
#define LOC_NO_ROOM  0xFF

/* -------------------------------------------------------------------------
 * Types
 * ---------------------------------------------------------------------- */

/** Per-room tracking state (internal, but exposed for stack allocation). */
typedef struct
{
    int16_t  rssi_filtered;   /**< IIR-filtered RSSI (×1, integer dBm)   */
    bool     seen_this_scan;  /**< was this room seen in the current scan */
} loc_room_state_t;

/** Location engine state. */
typedef struct
{
    const config_store_t *cfg;
    loc_room_state_t      rooms[CONFIG_MAX_ROOMS];
    uint8_t               current_room;    /**< best room index, or LOC_NO_ROOM */
    int8_t                current_rssi;    /**< filtered RSSI of current room    */
    uint8_t               miss_count;      /**< consecutive scans with no match  */
    bool                  signal_low;      /**< true if miss_count >= threshold  */
} loc_state_t;

/* -------------------------------------------------------------------------
 * API
 * ---------------------------------------------------------------------- */

/**
 * \brief Initialize the location engine.
 * \param state  Caller-owned state struct.
 * \param cfg    Pointer to initialized config_store (must outlive state).
 */
void loc_init(loc_state_t *state, const config_store_t *cfg);

/**
 * \brief Feed one beacon observation from the current scan.
 *
 * Call this once per OK+DISC record that passes the Factory ID + UUID filter.
 * The engine matches Major/Minor against config rooms and updates RSSI.
 *
 * \param state  Engine state.
 * \param major  iBeacon Major field (e.g. 0x0B01).
 * \param minor  iBeacon Minor field (e.g. 0x0002).
 * \param rssi   Measured RSSI in dBm (negative, e.g. -66).
 */
void loc_feed_beacon(loc_state_t *state, uint16_t major, uint16_t minor, int8_t rssi);

/**
 * \brief Signal end of one scan cycle (called after OK+DISCE).
 *
 * This triggers the room decision:
 * - Selects the room with highest filtered RSSI among those seen (F1.4).
 * - If no room was seen, increments miss_count (F1.3).
 * - Resets per-scan flags for the next cycle.
 */
void loc_end_scan(loc_state_t *state);

/**
 * \brief Get the current best room index (0..4), or LOC_NO_ROOM if none.
 */
uint8_t loc_get_room(const loc_state_t *state);

/**
 * \brief Get the filtered RSSI of the current room (dBm).
 * Returns 0 if no room is active.
 */
int8_t loc_get_rssi(const loc_state_t *state);

/**
 * \brief Check if signal is low (3+ missed scans — F1.3).
 */
bool loc_is_signal_low(const loc_state_t *state);

/**
 * \brief Check if current room's RSSI is below the warning threshold (F3.5).
 */
bool loc_is_rssi_weak(const loc_state_t *state);

/**
 * \brief Get the filtered RSSI for a specific room index (0-based).
 *
 * Returns the IIR-filtered RSSI for the given room regardless of whether
 * it is currently the "best" room. Useful for reading proximity to the
 * FSM's target beacon independently of which room is closest overall.
 *
 * \return Filtered RSSI in dBm, or -100 if index is invalid / room never seen.
 */
int8_t loc_get_room_rssi(const loc_state_t *state, uint8_t room_index);

#endif /* LOCATION_ENGINE_H */
