/*! ***************************************************************************
 * \brief  lock_drv.h — Layer 4 Servo backend (extends hal/lock.h)
 * \file   lock_drv.h
 *
 * Servo-specific extension of the portable hal/lock.h contract.
 * The 4 portable functions (init/open/close/is_open) come from hal/lock.h.
 * This header adds ONE servo-only function: lock_drv_set_pulse() for
 * bench calibration. The solenoid backend does NOT provide it — so callers
 * that use set_pulse are servo-only (bench test main.c), never the FSM.
 *
 * Backend: eFlexPWM0 submodule 1, channel A, pin P3_8 (J3 pin 11), 50 Hz.
 *
 * Dependencies: hal/lock.h, <MCXA153.h>
 * Depended on by: kidbytes_lock bench test main.c (calibration only)
 *
 * FR coverage: F4.1 (open within 500 ms), F4.2 (hold closed), F4.4 (admin_unlock)
 *****************************************************************************/
#ifndef LOCK_DRV_H
#define LOCK_DRV_H

#include <stdint.h>
#include <stdbool.h>
#include "lock.h"   /* portable contract: init/open/close/is_open */

/* -------------------------------------------------------------------------
 * Constants
 * ---------------------------------------------------------------------- */

/** Servo pulse widths in timer counts (at 750 kHz count frequency).
 *  1 ms = 750 counts, 1.5 ms = 1125 counts, 2 ms = 1500 counts.
 */
#define LOCK_PULSE_CLOSED   750    /**< 1.0 ms → latch engaged   */
#define LOCK_PULSE_CENTER   1125   /**< 1.5 ms → center position */
#define LOCK_PULSE_OPEN     1500   /**< 2.0 ms → latch released  */

/** PWM period in counts: 750 kHz / 50 Hz = 15000 counts = 20 ms */
#define LOCK_PWM_PERIOD     15000

/** Maximum latch travel time (F4.1: within 500 ms) */
#define LOCK_TRAVEL_MS      500

/* -------------------------------------------------------------------------
 * Servo-only API (extends the portable hal/lock.h contract)
 * The 4 portable functions are declared in hal/lock.h:
 *   lock_drv_init(), lock_drv_open(), lock_drv_close(), lock_drv_is_open()
 * ---------------------------------------------------------------------- */

/**
 * \brief Set an arbitrary pulse width (servo calibration only).
 * \param pulse_counts  Pulse width in timer counts (750–1500 range).
 *
 * NOT part of the portable contract. Solenoid backend does not provide it.
 * Used only by the bench-test main.c for calibration.
 */
void lock_drv_set_pulse(uint16_t pulse_counts);

#endif /* LOCK_DRV_H */
