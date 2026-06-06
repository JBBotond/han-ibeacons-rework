/*! ***************************************************************************
 * \brief  lock_drv_solenoid.c — Layer 4 Lock Driver (solenoid backend)
 * \file   lock_drv_solenoid.c
 *
 * Implements the hal/lock.h contract using a solenoid lock on GPIO2 pin 7
 * (P2_7). On/off actuator — no PWM. Adapted from Botond's solenoid.c, but
 * wrapped behind the portable lock.h API so the FSM (Layer 5) is untouched.
 *
 * Pin: P2_7, GPIO2, MUX Alt 0 (plain GPIO output).
 *
 * Polarity (SOLENOID_ACTIVE_HIGH):
 *   A solenoid lock is driven through a MOSFET/transistor. The MCU pin sets
 *   the gate. Default here = ACTIVE_HIGH:
 *     OPEN  → pin HIGH → solenoid energized → bolt retracts → latch released
 *     CLOSE → pin LOW  → solenoid de-energized → spring extends bolt → locked
 *   This is FAIL-SECURE: power loss → de-energized → stays LOCKED.
 *   If your driver board is inverted, flip SOLENOID_ACTIVE_HIGH to 0.
 *
 * WARNING (reported gap): the switch from servo to solenoid was a hardware
 * change NOT captured in the functional requirements. F4 in the SRS still
 * says "drive the lock actuator" without naming the actuator — both servo
 * and solenoid satisfy it, but the SRS/traceability matrix should be updated
 * to record which actuator the final box ships with.
 *
 * Dependencies: <MCXA153.h>, hal/lock.h
 * Depended on by: main_fsm (Layer 5) — UNCHANGED, calls same 4 functions.
 *
 * FR coverage: F4.1 (open), F4.2 (hold closed), F4.4 (admin_unlock)
 *****************************************************************************/
#include "lock.h"
#include <MCXA153.h>

/* -------------------------------------------------------------------------
 * Configuration
 * ---------------------------------------------------------------------- */
#define SOLENOID_PORT_PIN   7          /**< P2_7 */
#define SOLENOID_ACTIVE_HIGH 1         /**< 1 = HIGH energizes (fail-secure) */

/* -------------------------------------------------------------------------
 * State
 * ---------------------------------------------------------------------- */
static bool g_is_open = false;

/* -------------------------------------------------------------------------
 * Internal helpers
 * ---------------------------------------------------------------------- */
static inline void solenoid_energize(void)
{
#if SOLENOID_ACTIVE_HIGH
    GPIO2->PSOR = (1U << SOLENOID_PORT_PIN);  /* pin HIGH */
#else
    GPIO2->PCOR = (1U << SOLENOID_PORT_PIN);  /* pin LOW  */
#endif
}

static inline void solenoid_deenergize(void)
{
#if SOLENOID_ACTIVE_HIGH
    GPIO2->PCOR = (1U << SOLENOID_PORT_PIN);  /* pin LOW  */
#else
    GPIO2->PSOR = (1U << SOLENOID_PORT_PIN);  /* pin HIGH */
#endif
}

/* -------------------------------------------------------------------------
 * HAL contract implementation (same signatures as lock_drv.c / hal/lock.h)
 * ---------------------------------------------------------------------- */

void lock_drv_init(void)
{
    /* Enable PORT2 + GPIO2 clocks */
    MRCC0->MRCC_GLB_CC0_SET = MRCC_MRCC_GLB_CC0_PORT2(1);
    MRCC0->MRCC_GLB_CC1_SET = MRCC_MRCC_GLB_CC1_GPIO2(1);

    /* Release from reset */
    MRCC0->MRCC_GLB_RST0_SET = MRCC_MRCC_GLB_RST0_PORT2(1);
    MRCC0->MRCC_GLB_RST1_SET = MRCC_MRCC_GLB_RST1_GPIO2(1);

    /* P2_7 = plain GPIO output (MUX Alt 0), locked */
    PORT2->PCR[SOLENOID_PORT_PIN] = PORT_PCR_LK(1) | PORT_PCR_MUX(0);

    /* Set direction to output */
    GPIO2->PDDR |= (1U << SOLENOID_PORT_PIN);

    /* Boot state: CLOSED (de-energized = fail-secure) */
    solenoid_deenergize();
    g_is_open = false;
}

void lock_drv_open(void)
{
    /* F4.1: energize → bolt retracts → latch released */
    solenoid_energize();
    g_is_open = true;
}

void lock_drv_close(void)
{
    /* F4.2: de-energize → spring extends bolt → locked */
    solenoid_deenergize();
    g_is_open = false;
}

bool lock_drv_is_open(void)
{
    return g_is_open;
}
