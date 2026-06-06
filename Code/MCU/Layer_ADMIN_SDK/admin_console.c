/*! ***************************************************************************
 * \brief  admin_console.c — Layer 5 Admin Console implementation (F5.1–F5.4)
 * \file   admin_console.c
 *
 * A tiny, non-blocking line-oriented REPL over the LPUART0 debug serial.
 * Bytes arrive one at a time via admin_console_feed(); a full line is then
 * either matched against the secret (LOCKED) or parsed as a command (ACTIVE).
 *
 * Commands (ACTIVE state):
 *   help                         list commands
 *   show                         print rooms + route + secret
 *   room <i> <major> <minor> <ans>   set room slot i (F5.2)
 *   route <a> <b> ...            set visit order, indices 0..N-1 (F5.2)
 *   secret <str>                 change admin secret (F5.1)
 *   unlock                       open the lock now (F4.4 / F5.3)
 *   save                         persist a config-change marker to NVM (F7.2)
 *   exit                         leave admin, lock closes (F5.4)
 *
 * All output uses printf (already retargeted to LPUART0). The console never
 * blocks: it only acts when a newline byte arrives.
 *****************************************************************************/
#include "admin_console.h"
#include "main_fsm.h"
#include "log.h"
#include "nvm_drv.h"
#include "ble_drv.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* -------------------------------------------------------------------------
 * State
 * ---------------------------------------------------------------------- */
typedef enum
{
    ADMIN_LOCKED = 0,   /**< Waiting for the secret (F5.1)         */
    ADMIN_ACTIVE = 1    /**< Authenticated — commands accepted      */
} admin_mode_t;

static config_store_t *g_cfg   = 0;
static admin_mode_t     g_mode = ADMIN_LOCKED;
static char             g_line[ADMIN_LINE_MAX + 1];
static uint8_t          g_len  = 0;

/* -------------------------------------------------------------------------
 * Discovery print callback — handed to ble_drv when "scan on" is typed.
 * Prints every KidBytes beacon the box sees (matched or not) so the operator
 * can read the Major/Minor/RSSI list straight from the box's serial/GUI.
 * ---------------------------------------------------------------------- */
static void admin_discovery_print(uint16_t major, uint16_t minor, int8_t rssi)
{
    /* printf is retargeted to LPUART0 — same line the GUI now displays. */
    printf("DISCOVER major=0x%04X minor=0x%04X rssi=%ddBm\r\n",
           (unsigned)major, (unsigned)minor, (int)rssi);
}

/* -------------------------------------------------------------------------
 * Small helpers
 * ---------------------------------------------------------------------- */

/* Parse an unsigned value, accepting both 0x-hex and decimal. */
static uint32_t parse_uint(const char *s)
{
    return (uint32_t)strtoul(s, 0, 0);  /* base 0 = auto 0x / decimal */
}

/* Split g_line into argv[] on spaces (in place). Returns argc. */
static int tokenize(char *line, char *argv[], int max_args)
{
    int argc = 0;
    char *p = line;
    while (*p && argc < max_args)
    {
        while (*p == ' ' || *p == '\t') *p++ = '\0';
        if (*p == '\0') break;
        argv[argc++] = p;
        while (*p && *p != ' ' && *p != '\t') p++;
    }
    return argc;
}

static void print_banner(void)
{
    printf("\r\n=== KidBytes ADMIN (UART console) ===\r\n");
    printf("Type 'help' for commands, 'exit' to leave.\r\n> ");
}

static void cmd_show(void)
{
    uint8_t n = config_get_room_count(g_cfg);
    uint8_t steps = config_get_route_count(g_cfg);

    printf("beacons: %u\r\n", (unsigned)n);
    for (uint8_t i = 0; i < n; i++)
    {
        const config_room_t *r = config_get_room(g_cfg, i);
        if (r != 0)
        {
            printf("  [%u] major=0x%04X minor=0x%04X\r\n",
                   (unsigned)i, (unsigned)r->major, (unsigned)r->minor);
        }
    }
    printf("route: %u steps\r\n", (unsigned)steps);
    for (uint8_t i = 0; i < steps; i++)
    {
        uint8_t b = config_get_route_beacon(g_cfg, i);
        if (config_get_route_type(g_cfg, i) == BEACON_GAME)
        {
            printf("  step %u: beacon %u GAME puzzle=%u\r\n",
                   (unsigned)i, (unsigned)b,
                   (unsigned)config_get_route_puzzle(g_cfg, i));
        }
        else
        {
            printf("  step %u: beacon %u WAYPOINT\r\n",
                   (unsigned)i, (unsigned)b);
        }
    }
    printf("secret: \"%s\"\r\n", config_get_secret(g_cfg));
}

static void cmd_help(void)
{
    printf("commands:\r\n");
    printf("  show\r\n");
    printf("  room <i> <major> <minor> <answer>\r\n");
    printf("  step <pos> <beacon> game <puzzle> | wp   (route step role)\r\n");
    printf("  clearroute          (empty the route)\r\n");
    printf("  secret <str>\r\n");
    printf("  scan on|off         (print every beacon the box sees)\r\n");
    printf("  unlock\r\n");
    printf("  save\r\n");
    printf("  factory             (erase saved config + session NVM)\r\n");
    printf("  exit\r\n");
}

/* -------------------------------------------------------------------------
 * Command dispatch (ACTIVE state)
 * ---------------------------------------------------------------------- */
static void run_command(char *line)
{
    char *argv[8];
    int argc = tokenize(line, argv, 8);

    if (argc == 0) { printf("> "); return; }

    if (strcmp(argv[0], "help") == 0)
    {
        cmd_help();
    }
    else if (strcmp(argv[0], "show") == 0)
    {
        cmd_show();
    }
    else if (strcmp(argv[0], "room") == 0)
    {
        if (argc < 5) { printf("usage: room <i> <major> <minor> <answer>\r\n"); }
        else
        {
            config_room_t r;
            uint8_t idx = (uint8_t)parse_uint(argv[1]);
            r.major = (uint16_t)parse_uint(argv[2]);
            r.minor = (uint16_t)parse_uint(argv[3]);
            strncpy(r.answer, argv[4], CONFIG_ANSWER_LEN);
            r.answer[CONFIG_ANSWER_LEN] = '\0';

            if (config_set_room(g_cfg, idx, &r))
            {
                printf("ok: room[%u] = 0x%04X:0x%04X \"%s\"\r\n",
                       (unsigned)idx, (unsigned)r.major, (unsigned)r.minor, r.answer);
                LogWithNum(LOG_SYS_FSM, LOG_INFO, "admin set room=", idx);
            }
            else { printf("err: bad room index (0..%u)\r\n",
                          (unsigned)(config_get_room_count(g_cfg) - 1)); }
        }
    }
    else if (strcmp(argv[0], "step") == 0)
    {
        /* step <pos> <beacon> game <puzzle>   OR   step <pos> <beacon> wp */
        if (argc < 4) { printf("usage: step <pos> <beacon> game <puzzle> | wp\r\n"); }
        else
        {
            uint8_t pos    = (uint8_t)parse_uint(argv[1]);
            uint8_t beacon = (uint8_t)parse_uint(argv[2]);
            bool    is_game = (strcmp(argv[3], "game") == 0);
            uint8_t pid    = (is_game && argc >= 5) ? (uint8_t)parse_uint(argv[4]) : 0;

            if (config_set_route_step(g_cfg, pos, beacon, is_game, pid))
            {
                if (is_game)
                    printf("ok: step %u = beacon %u GAME puzzle %u\r\n",
                           (unsigned)pos, (unsigned)beacon, (unsigned)pid);
                else
                    printf("ok: step %u = beacon %u WAYPOINT\r\n",
                           (unsigned)pos, (unsigned)beacon);
                LogWithNum(LOG_SYS_FSM, LOG_INFO, "admin set step=", pos);
            }
            else { printf("err: bad pos/beacon (pos<%u beacon<%u)\r\n",
                          (unsigned)CONFIG_MAX_ROUTE, (unsigned)CONFIG_MAX_ROOMS); }
        }
    }
    else if (strcmp(argv[0], "clearroute") == 0)
    {
        config_clear_route(g_cfg);
        printf("ok: route cleared\r\n");
        Log(LOG_SYS_FSM, LOG_INFO, "admin cleared route");
    }
    else if (strcmp(argv[0], "secret") == 0)
    {
        if (argc < 2) { printf("usage: secret <str>\r\n"); }
        else if (config_set_secret(g_cfg, argv[1]))
        {
            printf("ok: secret updated\r\n");
            Log(LOG_SYS_FSM, LOG_INFO, "admin changed secret");
        }
        else { printf("err: secret too long (max %u)\r\n", (unsigned)CONFIG_SECRET_LEN); }
    }
    else if (strcmp(argv[0], "scan") == 0)
    {
        if (argc >= 2 && strcmp(argv[1], "on") == 0)
        {
            ble_drv_set_discovery(admin_discovery_print);
            printf("ok: discovery ON — beacons print as 'DISCOVER ...'\r\n");
            Log(LOG_SYS_SENSOR, LOG_INFO, "admin discovery ON");
        }
        else if (argc >= 2 && strcmp(argv[1], "off") == 0)
        {
            ble_drv_set_discovery(0);
            printf("ok: discovery OFF\r\n");
            Log(LOG_SYS_SENSOR, LOG_INFO, "admin discovery OFF");
        }
        else { printf("usage: scan on|off\r\n"); }
    }
    else if (strcmp(argv[0], "unlock") == 0)
    {
        fsm_event_t evt = { .type = EVT_ADMIN_UNLOCK, .timestamp = 0 };
        fsm_post_event(&evt);
        printf("ok: unlock requested\r\n");
    }
    else if (strcmp(argv[0], "save") == 0)
    {
        /* Persist the WHOLE live config (rooms + route + secret) to its own
         * NVM sector so the venue setup survives a power cut (F7.3). */
        if (nvm_drv_config_write(g_cfg, sizeof(config_store_t)))
        {
            printf("ok: config saved to NVM (survives power cut)\r\n");
            Log(LOG_SYS_FSM, LOG_INFO, "admin saved config blob");
        }
        else { printf("err: config NVM write failed\r\n"); }
    }
    else if (strcmp(argv[0], "factory") == 0)
    {
        /* Wipe the saved venue config AND the session region, so the box
         * boots clean on factory defaults. Use when the saved blob is wrong. */
        bool ok_cfg = nvm_drv_config_erase();
        bool ok_ses = nvm_drv_erase_all();
        if (ok_cfg && ok_ses)
        {
            printf("ok: NVM erased (config + session). Reset to apply defaults.\r\n");
            Log(LOG_SYS_FSM, LOG_WARN, "admin factory erase");
        }
        else { printf("err: erase failed (cfg=%d ses=%d)\r\n", (int)ok_cfg, (int)ok_ses); }
    }
    else if (strcmp(argv[0], "exit") == 0)
    {
        fsm_event_t evt = { .type = EVT_ADMIN_EXIT, .timestamp = 0 };
        fsm_post_event(&evt);
        g_mode = ADMIN_LOCKED;
        printf("admin session closed. lock re-secured.\r\n");
        return;  /* no prompt — we're locked again */
    }
    else
    {
        printf("unknown: %s (try 'help')\r\n", argv[0]);
    }

    printf("> ");
}

/* -------------------------------------------------------------------------
 * Line completion handler
 * ---------------------------------------------------------------------- */
static void process_line(void)
{
    g_line[g_len] = '\0';

    if (g_mode == ADMIN_LOCKED)
    {
        /* F5.1: only the secret unlocks the console. */
        if (g_len > 0 && strcmp(g_line, config_get_secret(g_cfg)) == 0)
        {
            g_mode = ADMIN_ACTIVE;
            /* Tell the FSM to enter ADMIN super-state (pauses the hunt). */
            fsm_event_t evt = { .type = EVT_ADMIN_ENTER, .timestamp = 0 };
            fsm_post_event(&evt);
            Log(LOG_SYS_FSM, LOG_INFO, "admin authenticated");
            print_banner();
        }
        /* Wrong/empty line while locked: stay silent (don't leak the gate). */
    }
    else
    {
        run_command(g_line);
    }

    g_len = 0;
}

/* -------------------------------------------------------------------------
 * Public API
 * ---------------------------------------------------------------------- */

void admin_console_init(config_store_t *cfg)
{
    g_cfg  = cfg;
    g_mode = ADMIN_LOCKED;
    g_len  = 0;
}

void admin_console_feed(uint8_t c)
{
    if (g_cfg == 0) return;  /* not initialized */

    if (c == '\r' || c == '\n')
    {
        if (g_len > 0 || g_mode == ADMIN_ACTIVE)
        {
            process_line();
        }
        return;
    }

    /* Simple backspace handling for interactive terminals. */
    if ((c == 0x08 || c == 0x7F) && g_len > 0)
    {
        g_len--;
        return;
    }

    if (g_len < ADMIN_LINE_MAX)
    {
        g_line[g_len++] = (char)c;
    }
    /* Overflow bytes are dropped; line stays bounded (no buffer overrun). */
}

bool admin_console_is_active(void)
{
    return (g_mode == ADMIN_ACTIVE);
}
