/*! ***************************************************************************
 * \brief  ble_drv — Layer 4 Driver (MCXA153-specific)
 * \file   ble_drv.h
 *
 * Implements the BLE HAL (ble.h) using HM-10 CC2541 over LPUART2 at 9600 bps.
 * Parses the AT+DISI? token stream and feeds location_engine.
 *
 * Token format (no CR/LF — concatenated):
 *   OK+DISIS                          (8 bytes — scan started)
 *   OK+DISC:<78-byte record>          (78 bytes per device)
 *   OK+DISCE                          (8 bytes — scan ended)
 *
 * Record layout (78 bytes total, starting after "OK+DISC:"):
 *   [0..7]   Factory ID  (e.g. "4C000215" = Apple iBeacon)
 *   [8]      ':'
 *   [9..40]  UUID (32 hex chars)
 *   [41]     ':'
 *   [42..51] Major(4) + Minor(4) + TxPower(2)
 *   [52]     ':'
 *   [53..64] MAC (12 hex chars)
 *   [65]     ':'
 *   [66..69] RSSI as signed decimal (e.g. "-066")
 *
 * BUT measured from the full "OK+DISC:..." token (78 bytes):
 *   Factory  @ offset 8, length 8
 *   UUID     @ offset 17, length 32
 *   Major    @ offset 50, length 4
 *   Minor    @ offset 54, length 4
 *   TxPower  @ offset 58, length 2
 *   RSSI     @ offset 74, to end (signed decimal with leading '-')
 *
 * Dependencies: lpuart2.h, location_engine.h, <MCXA153.h>
 * Depended on by: scan_task (Layer 5)
 *
 * FR coverage: F1.1 (scan cycle), F1.2 (Major/Minor resolution)
 *****************************************************************************/
#ifndef BLE_DRV_H
#define BLE_DRV_H

#include <stdint.h>
#include <stdbool.h>
#include "location_engine.h"

/* -------------------------------------------------------------------------
 * Constants
 * ---------------------------------------------------------------------- */

#define BLE_SCAN_PERIOD_MS   250   /**< F1.1: scan every 250 ms */

/* -------------------------------------------------------------------------
 * Discovery hook (optional — for the admin "scan" command).
 *
 * When discovery is ON, ble_drv_poll calls the registered callback for EVERY
 * KidBytes-UUID beacon it sees (matched to a room or not), so an operator can
 * find what Major/Minor values exist before configuring the route. This is
 * pure observation: the normal match path (loc_feed_beacon) is unchanged, and
 * the callback lives in the caller so this driver keeps zero log dependency.
 * ---------------------------------------------------------------------- */
typedef void (*ble_discovery_cb_t)(uint16_t major, uint16_t minor, int8_t rssi);

/* -------------------------------------------------------------------------
 * Types
 * ---------------------------------------------------------------------- */

/** BLE driver state. */
typedef struct
{
    loc_state_t  *loc;             /**< pointer to location engine state   */
    char          rx_buf[512];     /**< accumulation buffer for UART bytes */
    uint16_t      rx_len;          /**< current bytes in rx_buf            */
    bool          scan_active;     /**< true between DISIS and DISCE       */
    uint32_t      last_scan_ms;    /**< timestamp of last scan trigger     */
    uint8_t       beacons_total;   /**< total BLE devices in last scan     */
    uint8_t       beacons_matched; /**< iBeacons matching our UUID         */
} ble_drv_state_t;

/* -------------------------------------------------------------------------
 * API
 * ---------------------------------------------------------------------- */

/**
 * \brief Initialize the BLE driver (LPUART2 + HM-10 probe).
 * \param state  Caller-owned state struct.
 * \param loc    Pointer to initialized location_engine state.
 *
 * Initializes LPUART2 at 9600 bps and verifies HM-10 responds to AT.
 * \return true if HM-10 responded, false if timeout (continues anyway).
 */
bool ble_drv_init(ble_drv_state_t *state, loc_state_t *loc);

/**
 * \brief Trigger a new scan (sends AT+DISI? to HM-10).
 *
 * Call this periodically from scan_task (every BLE_SCAN_PERIOD_MS).
 * Non-blocking — results arrive asynchronously via ble_drv_poll().
 */
void ble_drv_start_scan(ble_drv_state_t *state);

/**
 * \brief Poll for incoming UART data and process tokens.
 *
 * Call this frequently from the main loop (or scan_task).
 * Consumes bytes from LPUART2 RX FIFO, parses tokens, and feeds
 * location_engine automatically.
 *
 * \return true if a complete scan cycle finished (OK+DISCE received).
 */
bool ble_drv_poll(ble_drv_state_t *state);

/**
 * \brief Check if a scan is currently in progress.
 */
bool ble_drv_is_scanning(const ble_drv_state_t *state);

/**
 * \brief Get number of matched iBeacons from the last completed scan.
 */
uint8_t ble_drv_get_matched_count(const ble_drv_state_t *state);

/**
 * \brief Register (or clear) the discovery callback.
 * \param cb  Callback invoked for every KidBytes-UUID beacon seen, or NULL
 *            to disable discovery. Default is NULL (off) — normal matching
 *            is unaffected either way.
 */
void ble_drv_set_discovery(ble_discovery_cb_t cb);

#endif /* BLE_DRV_H */
