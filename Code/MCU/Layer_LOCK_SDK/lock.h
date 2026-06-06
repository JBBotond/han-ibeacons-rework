/*! ***************************************************************************
 * \brief  lock.h — Layer 3 HAL: portable lock contract
 * \file   hal/lock.h
 *
 * The vendor-independent contract every lock backend must implement.
 * This is the swap point: the FSM (Layer 5) depends ONLY on these 4
 * functions, never on the actuator type. Swapping servo ↔ solenoid ↔ relay
 * changes only which Layer 4 .c file is compiled — nothing above Layer 4.
 *
 * Backends that implement this contract:
 *   - lock_drv.c          → servo via eFlexPWM0  (P3_8, 50 Hz PWM)
 *   - lock_drv_solenoid.c → solenoid via GPIO2   (P2_7, on/off)
 *
 * Rule (architecture Layer 3): pure header, only <stdint.h>/<stdbool.h>,
 * zero #include of other HAL headers, zero MCU-specific symbols.
 *
 * FR coverage: F4.1 (open within 500 ms), F4.2 (hold closed), F4.4 (admin_unlock)
 *****************************************************************************/
#ifndef HAL_LOCK_H
#define HAL_LOCK_H

#include <stdint.h>
#include <stdbool.h>

/**
 * \brief Initialize the lock hardware. Starts in CLOSED position.
 */
void lock_drv_init(void);

/**
 * \brief Drive the lock to the OPEN position (F4.1).
 *        Latch released within 500 ms.
 */
void lock_drv_open(void);

/**
 * \brief Drive the lock to the CLOSED position (F4.2).
 *        Latch engaged. Default state at boot.
 */
void lock_drv_close(void);

/**
 * \brief Check if the lock is currently OPEN.
 * \return true if open, false if closed.
 */
bool lock_drv_is_open(void);

#endif /* HAL_LOCK_H */
