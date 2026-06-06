/*! ***************************************************************************
 * \brief  config_store — Layer 7 Model implementation
 * \file   config_store.c
 *
 * Pure C, no MCU dependencies. Compile with gcc on any host.
 *****************************************************************************/
#include "config_store.h"
#include <string.h>

/* -------------------------------------------------------------------------
 * Factory defaults
 * ---------------------------------------------------------------------- */

void config_init(config_store_t *cfg)
{
    if (cfg == NULL) return;

    memset(cfg, 0, sizeof(config_store_t));

    /* ---- Beacons (distinct Minors 0x0001..0x000A → slots 0..9) ----
     * All share Major 0x0B01. answer is unused by the route flow but kept. */
    cfg->room_count = 10;
    for (uint8_t i = 0; i < CONFIG_MAX_ROOMS; i++)
    {
        cfg->rooms[i].major = 0x0B01;
        cfg->rooms[i].minor = (uint16_t)(i + 1);   /* slot i → Minor i+1 */
        cfg->rooms[i].answer[0] = '\0';
        cfg->rooms[i].type      = BEACON_WAYPOINT; /* legacy field, unused now */
        cfg->rooms[i].puzzle_id = 0;
    }

    /* ---- Company route map (16 steps; role lives on the STEP) ----
     * Beacon slot = Minor-1. G = game (with puzzle id), W = waypoint.
     *   step beacon(Minor)  role   puzzle
     *    0     0 (1)        GAME    0  RPS
     *    1     1 (2)        WAYPOINT
     *    2     2 (3)        WAYPOINT
     *    3     3 (4)        WAYPOINT
     *    4     4 (5)        WAYPOINT
     *    5     5 (6)        GAME    1  QUIZ
     *    6     4 (5)        WAYPOINT
     *    7     3 (4)        WAYPOINT
     *    8     2 (3)        GAME    2  MOLE
     *    9     3 (4)        WAYPOINT
     *   10     6 (7)        WAYPOINT
     *   11     7 (8)        WAYPOINT
     *   12     8 (9)        WAYPOINT
     *   13     9 (A)        GAME    3  SIMON
     *   14     8 (9)        WAYPOINT
     *   15     7 (8)        GAME    4  QUIZ2
     */
    config_set_route_step(cfg,  0, 0, true,  0);
    config_set_route_step(cfg,  1, 1, false, 0);
    config_set_route_step(cfg,  2, 2, false, 0);
    config_set_route_step(cfg,  3, 3, false, 0);
    config_set_route_step(cfg,  4, 4, false, 0);
    config_set_route_step(cfg,  5, 5, true,  1);
    config_set_route_step(cfg,  6, 4, false, 0);
    config_set_route_step(cfg,  7, 3, false, 0);
    config_set_route_step(cfg,  8, 2, true,  2);
    config_set_route_step(cfg,  9, 3, false, 0);
    config_set_route_step(cfg, 10, 6, false, 0);
    config_set_route_step(cfg, 11, 7, false, 0);
    config_set_route_step(cfg, 12, 8, false, 0);
    config_set_route_step(cfg, 13, 9, true,  3);
    config_set_route_step(cfg, 14, 8, false, 0);
    config_set_route_step(cfg, 15, 7, true,  4);

    /* Default admin secret */
    strncpy(cfg->secret, "ADMIN", CONFIG_SECRET_LEN);
    cfg->secret[CONFIG_SECRET_LEN] = '\0';
}

/* -------------------------------------------------------------------------
 * Getters (read-only access)
 * ---------------------------------------------------------------------- */

uint8_t config_get_room_count(const config_store_t *cfg)
{
    if (cfg == NULL) return 0;
    return cfg->room_count;
}

const config_room_t *config_get_room(const config_store_t *cfg, uint8_t index)
{
    if (cfg == NULL) return NULL;
    if (index >= cfg->room_count) return NULL;
    return &cfg->rooms[index];
}

const uint8_t *config_get_route(const config_store_t *cfg)
{
    if (cfg == NULL) return NULL;
    return cfg->route;
}

const char *config_get_secret(const config_store_t *cfg)
{
    if (cfg == NULL) return NULL;
    return cfg->secret;
}

/* -------------------------------------------------------------------------
 * Setters (admin CRUD — F5.2)
 * ---------------------------------------------------------------------- */

bool config_set_room(config_store_t *cfg, uint8_t index, const config_room_t *room)
{
    if (cfg == NULL || room == NULL) return false;
    /* Bound on the physical slot array, NOT the active route length. Gating on
     * room_count created a trap: once route shrank room_count to 1, you could
     * never populate slots 1..4 again. CONFIG_MAX_ROOMS is the real limit. */
    if (index >= CONFIG_MAX_ROOMS) return false;

    cfg->rooms[index].major = room->major;
    cfg->rooms[index].minor = room->minor;
    strncpy(cfg->rooms[index].answer, room->answer, CONFIG_ANSWER_LEN);
    cfg->rooms[index].answer[CONFIG_ANSWER_LEN] = '\0';
    /* type/puzzle_id are managed by config_set_game/_waypoint, not here, so a
     * plain "room" edit (major/minor/answer) never clobbers the beacon role. */

    return true;
}

bool config_set_secret(config_store_t *cfg, const char *secret)
{
    if (cfg == NULL || secret == NULL) return false;
    if (strlen(secret) > CONFIG_SECRET_LEN) return false;

    strncpy(cfg->secret, secret, CONFIG_SECRET_LEN);
    cfg->secret[CONFIG_SECRET_LEN] = '\0';

    return true;
}

bool config_set_route(config_store_t *cfg, const uint8_t *order, uint8_t count)
{
    if (cfg == NULL || order == NULL) return false;
    if (count == 0 || count > CONFIG_MAX_ROUTE) return false;

    /* Every step must point at a real beacon slot. */
    for (uint8_t i = 0; i < count; i++)
    {
        if (order[i] >= CONFIG_MAX_ROOMS) return false;
    }

    memcpy(cfg->route, order, count);
    cfg->route_count = count;
    /* Roles default to WAYPOINT unless set per-step elsewhere. */

    return true;
}

/* -------------------------------------------------------------------------
 * Route-step role API (role lives on the STEP, not the beacon)
 * ---------------------------------------------------------------------- */

uint8_t config_get_route_count(const config_store_t *cfg)
{
    if (cfg == NULL) return 0;
    return cfg->route_count;
}

uint8_t config_get_route_beacon(const config_store_t *cfg, uint8_t pos)
{
    if (cfg == NULL || pos >= cfg->route_count) return 0xFF;
    return cfg->route[pos];
}

beacon_type_t config_get_route_type(const config_store_t *cfg, uint8_t pos)
{
    if (cfg == NULL || pos >= cfg->route_count) return BEACON_WAYPOINT;
    return (beacon_type_t)cfg->route_type[pos];
}

uint8_t config_get_route_puzzle(const config_store_t *cfg, uint8_t pos)
{
    if (cfg == NULL || pos >= cfg->route_count) return 0;
    return cfg->route_puzzle[pos];
}

bool config_set_route_step(config_store_t *cfg, uint8_t pos, uint8_t beacon,
                           bool is_game, uint8_t puzzle_id)
{
    if (cfg == NULL) return false;
    if (pos >= CONFIG_MAX_ROUTE) return false;
    if (beacon >= CONFIG_MAX_ROOMS) return false;

    cfg->route[pos]        = beacon;
    cfg->route_type[pos]   = is_game ? BEACON_GAME : BEACON_WAYPOINT;
    cfg->route_puzzle[pos] = is_game ? puzzle_id : 0;

    /* Grow the route to include this step (steps set in order). */
    if (pos >= cfg->route_count) cfg->route_count = (uint8_t)(pos + 1);

    return true;
}

void config_clear_route(config_store_t *cfg)
{
    if (cfg == NULL) return;
    cfg->route_count = 0;
}
