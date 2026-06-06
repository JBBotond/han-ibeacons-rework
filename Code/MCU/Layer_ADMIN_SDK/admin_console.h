/*! ***************************************************************************
 * \brief  admin_console.h — Layer 5 Admin Console (F5.1–F5.4)
 * \file   admin_console.h
 *
 * Interactive text REPL that lets a venue operator reconfigure the box at
 * run time: edit the room->beacon map, reorder (or repeat) the route,
 * change the admin secret, and manually open the lock. This is the "proper"
 * Option-B admin path that backs the data-driven route in main_fsm.c.
 *
 * TRANSPORT (reported gap): F5.1 names "USB-CDC", but the MCXA153 SDK ships
 * NO USB-CDC stack — only the raw PERI_USB.h register header. This console
 * therefore runs over the EXISTING LPUART0 debug serial (the same COM port
 * already used for logs), which already has interrupt-driven RX. Behaviour
 * (secret gate, CRUD, unlock, exit) is identical; only the wire differs.
 * The SRS/traceability matrix should record UART-console instead of USB-CDC.
 *
 * It is fed one received byte at a time (admin_console_feed) so it never
 * blocks the cooperative main loop. A full line ('\n' or '\r') triggers
 * either a secret check (when LOCKED) or a command parse (when ACTIVE).
 *
 * Dependencies: config_store.h (edits), main_fsm.h (posts admin events)
 * Depended on by: main.c (the dispatcher feeds it serial bytes)
 *
 * FR coverage: F5.1 (secret entry), F5.2 (room/route CRUD),
 *              F5.3/F4.4 (manual unlock), F5.4 (exit)
 *****************************************************************************/
#ifndef ADMIN_CONSOLE_H
#define ADMIN_CONSOLE_H

#include <stdint.h>
#include <stdbool.h>
#include "config_store.h"

/** Max characters in one console input line (excluding NUL). */
#define ADMIN_LINE_MAX  48

/**
 * \brief Initialize the admin console.
 * \param cfg  Live config store the console edits (same instance the FSM
 *             borrows). Must outlive the console.
 *
 * Starts LOCKED: the box ignores commands until the F5.1 secret is typed.
 */
void admin_console_init(config_store_t *cfg);

/**
 * \brief Feed one received byte into the console line buffer (non-blocking).
 * \param c  Byte from serial RX.
 *
 * On end-of-line it either matches the secret (LOCKED) or runs a command
 * (ACTIVE). Safe to call from the main loop for every available RX byte.
 */
void admin_console_feed(uint8_t c);

/**
 * \brief Is the console currently in the ACTIVE (authenticated) state?
 * \return true while inside an admin session, false when LOCKED.
 */
bool admin_console_is_active(void);

#endif /* ADMIN_CONSOLE_H */
