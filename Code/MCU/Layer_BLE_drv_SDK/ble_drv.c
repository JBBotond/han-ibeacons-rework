/*! ***************************************************************************
 * \brief  ble_drv — Layer 4 Driver implementation
 * \file   ble_drv.c
 *
 * Token-based parser for HM-10 AT+DISI? responses.
 * Same logic as the Qt GUI parser (dialog.cpp), but in pure C on the MCU.
 *****************************************************************************/
#include "ble_drv.h"
#include "lpuart2.h"
#include <string.h>
#include <stdlib.h>

/* -------------------------------------------------------------------------
 * Token constants (from measured HM-10 output)
 * ---------------------------------------------------------------------- */
static const char TOKEN_DISIS[]   = "OK+DISIS";   /* 8 bytes */
static const char TOKEN_DISCE[]   = "OK+DISCE";   /* 8 bytes */
static const char TOKEN_DISC[]    = "OK+DISC:";   /* 8 bytes prefix */
static const char FACTORY_IBEACON[] = "4C000215";  /* Apple iBeacon */
static const char UUID_KIDBYTES[]   = "74278BDAB64445208F0C720EAF059935";

#define TOKEN_LEN     8
#define DISC_REC_LEN  78   /* full "OK+DISC:..." record length */

/* Field offsets within the 78-byte record */
#define OFF_FACTORY   8    /* 8 chars */
#define OFF_UUID      17   /* 32 chars */
#define OFF_MAJOR     50   /* 4 hex chars */
#define OFF_MINOR     54   /* 4 hex chars */
#define OFF_TXPOWER   58   /* 2 hex chars */
#define OFF_RSSI      74   /* signed decimal to end, e.g. "-066" */

/* -------------------------------------------------------------------------
 * Discovery callback (optional — off by default; see ble_drv.h).
 * ---------------------------------------------------------------------- */
static ble_discovery_cb_t g_discovery_cb = 0;

void ble_drv_set_discovery(ble_discovery_cb_t cb)
{
    g_discovery_cb = cb;
}

/* -------------------------------------------------------------------------
 * Helpers
 * ---------------------------------------------------------------------- */

/** Convert 4 hex ASCII chars to uint16_t. */
static uint16_t hex4_to_u16(const char *s)
{
    uint16_t val = 0;
    for (int i = 0; i < 4; i++)
    {
        val <<= 4;
        char c = s[i];
        if (c >= '0' && c <= '9')      val |= (uint16_t)(c - '0');
        else if (c >= 'A' && c <= 'F') val |= (uint16_t)(c - 'A' + 10);
        else if (c >= 'a' && c <= 'f') val |= (uint16_t)(c - 'a' + 10);
    }
    return val;
}

/** Parse signed decimal RSSI from position (e.g. ":-066" → -66). */
static int8_t parse_rssi(const char *s, uint16_t max_len)
{
    /* Skip leading ':' if present */
    if (*s == ':') { s++; max_len--; }

    int sign = 1;
    int val = 0;

    if (*s == '-') { sign = -1; s++; max_len--; }
    else if (*s == '+') { s++; max_len--; }

    while (max_len > 0 && *s >= '0' && *s <= '9')
    {
        val = val * 10 + (*s - '0');
        s++;
        max_len--;
    }

    int result = sign * val;

    /* Clamp to int8_t range */
    if (result < -127) result = -127;
    if (result > 0) result = 0;

    return (int8_t)result;
}

/** Send a string to HM-10 via LPUART2. */
static void hm10_send(const char *str)
{
    while (*str)
    {
        lpuart2_putchar(*str);
        str++;
    }
}

/* -------------------------------------------------------------------------
 * Init
 * ---------------------------------------------------------------------- */

bool ble_drv_init(ble_drv_state_t *state, loc_state_t *loc)
{
    if (state == NULL || loc == NULL) return false;

    memset(state, 0, sizeof(ble_drv_state_t));
    state->loc = loc;

    /* Initialize LPUART2 at 9600 bps (HM-10 default) */
    lpuart2_init(9600);

    /* Wait for HM-10 boot (it may send a banner after power-on) */
    extern volatile uint32_t ms;
    uint32_t start = ms;
    while ((ms - start) < 500) {}

    /* Flush any boot banner bytes */
    while (lpuart2_rxcnt() > 0)
    {
        lpuart2_getchar();
    }

    /* Probe: send "AT" and wait for "OK" (2 bytes, no CRLF) */
    hm10_send("AT");

    start = ms;
    uint8_t rx_count = 0;
    while ((ms - start) < 2000 && rx_count < 2)
    {
        if (lpuart2_rxcnt() > 0)
        {
            lpuart2_getchar();  /* consume 'O' then 'K' */
            rx_count++;
        }
    }

    return (rx_count >= 2);
}

/* -------------------------------------------------------------------------
 * Scan trigger
 * ---------------------------------------------------------------------- */

void ble_drv_start_scan(ble_drv_state_t *state)
{
    if (state == NULL) return;

    /* Clear buffer for new scan */
    state->rx_len = 0;
    state->beacons_total = 0;
    state->beacons_matched = 0;

    /* Send scan command (no \r\n — HM-10 doesn't need it) */
    hm10_send("AT+DISI?");
}

/* -------------------------------------------------------------------------
 * Token parser (called from main loop)
 * ---------------------------------------------------------------------- */

bool ble_drv_poll(ble_drv_state_t *state)
{
    if (state == NULL) return false;

    /* Drain LPUART2 RX FIFO into our buffer */
    while (lpuart2_rxcnt() > 0 && state->rx_len < sizeof(state->rx_buf) - 1)
    {
        state->rx_buf[state->rx_len] = (char)lpuart2_getchar();
        state->rx_len++;
    }

    /* Process tokens (same logic as Qt dialog.cpp::onSerialData) */
    while (state->rx_len >= TOKEN_LEN)
    {
        /* Check for OK+DISIS (scan started) */
        if (memcmp(state->rx_buf, TOKEN_DISIS, TOKEN_LEN) == 0)
        {
            state->scan_active = true;
            /* Consume 8 bytes */
            memmove(state->rx_buf, state->rx_buf + TOKEN_LEN, state->rx_len - TOKEN_LEN);
            state->rx_len -= TOKEN_LEN;
            continue;
        }

        /* Check for OK+DISCE (scan ended) */
        if (memcmp(state->rx_buf, TOKEN_DISCE, TOKEN_LEN) == 0)
        {
            state->scan_active = false;
            /* Signal end of scan to location engine */
            loc_end_scan(state->loc);
            /* Consume 8 bytes */
            memmove(state->rx_buf, state->rx_buf + TOKEN_LEN, state->rx_len - TOKEN_LEN);
            state->rx_len -= TOKEN_LEN;
            return true;  /* scan complete */
        }

        /* Check for OK+DISC: (device record, 78 bytes) */
        if (memcmp(state->rx_buf, TOKEN_DISC, TOKEN_LEN) == 0)
        {
            /* Need full 78 bytes before parsing */
            if (state->rx_len < DISC_REC_LEN) break;  /* wait for more data */

            state->beacons_total++;

            /* Check Factory ID (offset 8, 8 chars) */
            bool is_ibeacon = (memcmp(&state->rx_buf[OFF_FACTORY], FACTORY_IBEACON, 8) == 0);

            /* Check UUID (offset 17, 32 chars) */
            bool is_ours = is_ibeacon &&
                           (memcmp(&state->rx_buf[OFF_UUID], UUID_KIDBYTES, 32) == 0);

            if (is_ours)
            {
                /* Extract Major, Minor, RSSI */
                uint16_t major = hex4_to_u16(&state->rx_buf[OFF_MAJOR]);
                uint16_t minor = hex4_to_u16(&state->rx_buf[OFF_MINOR]);
                int8_t   rssi  = parse_rssi(&state->rx_buf[OFF_RSSI],
                                            (uint16_t)(DISC_REC_LEN - OFF_RSSI));

                /* Discovery hook: report EVERY KidBytes beacon seen, whether
                 * or not it maps to a configured room. Off by default, so the
                 * normal demo path is unchanged. */
                if (g_discovery_cb != 0)
                {
                    g_discovery_cb(major, minor, rssi);
                }

                /* Feed to location engine (the normal match path) */
                loc_feed_beacon(state->loc, major, minor, rssi);

                state->beacons_matched++;
            }

            /* Consume 78 bytes */
            memmove(state->rx_buf, state->rx_buf + DISC_REC_LEN, state->rx_len - DISC_REC_LEN);
            state->rx_len -= DISC_REC_LEN;
            continue;
        }

        /* Unknown byte at head — drop 1 and resync */
        memmove(state->rx_buf, state->rx_buf + 1, state->rx_len - 1);
        state->rx_len--;
    }

    return false;  /* scan not yet complete */
}

/* -------------------------------------------------------------------------
 * Getters
 * ---------------------------------------------------------------------- */

bool ble_drv_is_scanning(const ble_drv_state_t *state)
{
    if (state == NULL) return false;
    return state->scan_active;
}

uint8_t ble_drv_get_matched_count(const ble_drv_state_t *state)
{
    if (state == NULL) return 0;
    return state->beacons_matched;
}
