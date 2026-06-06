/*! ***************************************************************************
 * \brief  config_store — Layer 7 Model (host-testable)
 * \file   config_store.h
 *
 * Shared, read-mostly source of truth for the KidBytes Puzzle Box.
 * Holds: 5 rooms (Major/Minor/puzzle answer), route sequence, admin secret.
 *
 * Dependencies: <stdint.h>, <stdbool.h> only. No MCU calls.
 * Depended on by: location_engine, puzzle_engine, route_manager, log_model.
 *
 * FR coverage: F5 (admin CRUD), F7 (persistence contract — caller persists).
 *****************************************************************************/
#ifndef CONFIG_STORE_H
#define CONFIG_STORE_H

#include <stdint.h>
#include <stdbool.h>

/* -------------------------------------------------------------------------
 * Constants
 * ---------------------------------------------------------------------- */
#define CONFIG_MAX_ROOMS      16  /* distinct beacon slots (Minor 1..N)     */
#define CONFIG_MAX_ROUTE      32  /* route steps (a beacon may repeat)      */
#define CONFIG_DEFAULT_ROOMS  5   /* legacy default count (overridden below)*/
#define CONFIG_SECRET_LEN     8   /* max admin secret length (excl. NUL) */
#define CONFIG_ANSWER_LEN     8   /* max puzzle answer length (excl. NUL) */

/* -------------------------------------------------------------------------
 * Types
 * ---------------------------------------------------------------------- */

/** Role of a beacon along the route (item 2). */
typedef enum
{
    BEACON_WAYPOINT = 0,   /**< transition beacon: guide-through, NO game  */
    BEACON_GAME     = 1    /**< game beacon: arm + PLAY button + puzzle    */
} beacon_type_t;

/** One room/beacon in the route map. */
typedef struct
{
    uint16_t major;                         /**< iBeacon Major (building ID) */
    uint16_t minor;                         /**< iBeacon Minor (room ID)     */
    char     answer[CONFIG_ANSWER_LEN + 1]; /**< puzzle answer (NUL-term)    */
    uint8_t  type;                          /**< beacon_type_t: GAME/WAYPOINT */
    uint8_t  puzzle_id;                     /**< which puzzle (0..N) if GAME  */
} config_room_t;

/** Full configuration block. */
typedef struct
{
    config_room_t rooms[CONFIG_MAX_ROOMS];
    uint8_t       room_count;               /**< distinct beacons defined    */
    uint8_t       route[CONFIG_MAX_ROUTE];   /**< visit order (beacon indices)*/
    uint8_t       route_count;              /**< number of route steps       */
    uint8_t       route_type[CONFIG_MAX_ROUTE];   /**< per-step: GAME/WAYPOINT */
    uint8_t       route_puzzle[CONFIG_MAX_ROUTE]; /**< per-step: puzzle id     */
    char          secret[CONFIG_SECRET_LEN + 1]; /**< admin shared secret    */
} config_store_t;

/* -------------------------------------------------------------------------
 * API
 * ---------------------------------------------------------------------- */

/**
 * \brief Load factory defaults into the config store.
 *
 * Defaults: 5 rooms with Major=0x0B01, Minor=0x0001..0x0005,
 * route = {0,1,2,3,4}, secret = "ADMIN".
 */
void config_init(config_store_t *cfg);

/** \brief Get number of active rooms. */
uint8_t config_get_room_count(const config_store_t *cfg);

/** \brief Get pointer to room at index (0-based). NULL if out of range. */
const config_room_t *config_get_room(const config_store_t *cfg, uint8_t index);

/** \brief Get the route order array (length = room_count). */
const uint8_t *config_get_route(const config_store_t *cfg);

/** \brief Get the admin secret string. */
const char *config_get_secret(const config_store_t *cfg);

/**
 * \brief Set/update a room entry (admin CRUD — F5.2).
 * \return true on success, false if index >= room_count.
 */
bool config_set_room(config_store_t *cfg, uint8_t index, const config_room_t *room);

/**
 * \brief Set the admin secret (F5.1).
 * \return true on success, false if secret too long.
 */
bool config_set_secret(config_store_t *cfg, const char *secret);

/**
 * \brief Set the route order (F5.2).
 * \param order Array of room_count indices.
 * \return true on success, false if any index >= room_count.
 */
bool config_set_route(config_store_t *cfg, const uint8_t *order, uint8_t count);

/* -------------------------------------------------------------------------
 * Route-step role API (item 2 — role lives on the STEP, not the beacon, so
 * the same physical beacon can be a waypoint at one step and a game later).
 * ---------------------------------------------------------------------- */

/** \brief Number of steps in the route (route_count). */
uint8_t config_get_route_count(const config_store_t *cfg);

/** \brief Beacon index visited at route step `pos` (0xFF if out of range). */
uint8_t config_get_route_beacon(const config_store_t *cfg, uint8_t pos);

/** \brief Role of route step `pos` (BEACON_GAME or BEACON_WAYPOINT). */
beacon_type_t config_get_route_type(const config_store_t *cfg, uint8_t pos);

/** \brief Puzzle id launched at route step `pos` (valid when step is GAME). */
uint8_t config_get_route_puzzle(const config_store_t *cfg, uint8_t pos);

/**
 * \brief Define one route step: which beacon, its role, and (if GAME) puzzle.
 * \param pos       step index 0..CONFIG_MAX_ROUTE-1
 * \param beacon    beacon slot index 0..room_count-1
 * \param is_game   true = GAME step, false = WAYPOINT step
 * \param puzzle_id puzzle to launch when is_game (ignored otherwise)
 * \return false if pos/beacon out of range.
 *
 * Grows route_count to pos+1 when needed, so steps can be set in order.
 */
bool config_set_route_step(config_store_t *cfg, uint8_t pos, uint8_t beacon,
                           bool is_game, uint8_t puzzle_id);

/** \brief Reset the route to empty (route_count = 0). */
void config_clear_route(config_store_t *cfg);

#endif /* CONFIG_STORE_H */
