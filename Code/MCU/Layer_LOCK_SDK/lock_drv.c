/*! ***************************************************************************
 * \brief  lock_drv.c — Layer 4 Lock Driver (servo backend)
 * \file   lock_drv.c
 *
 * Implements the hal/lock.h contract using a hobby servo via eFlexPWM0,
 * submodule 1, channel A on P3_8 (J3 pin 11). This is the DEFAULT backend.
 * The alternative backend is lock_drv_solenoid.c (GPIO2_7 on/off).
 * Only ONE backend is compiled at a time — selected in CMakeLists.txt.
 *
 * Clock chain:
 *   main_clk = 96 MHz (FIRC FREQ_SEL=0b101)
 *   eFlexPWM0 clock = main_clk (fixed, see RM Figure 64)
 *   Prescaler = 128 → count freq = 96 MHz / 128 = 750 kHz
 *   VAL1 (period) = 15000 - 1 = 14999 → 750000 / 15000 = 50 Hz (20 ms)
 *   Pulse width:
 *     1.0 ms =  750 counts → CLOSED
 *     1.5 ms = 1125 counts → CENTER
 *     2.0 ms = 1500 counts → OPEN
 *
 * Edge-aligned PWM:
 *   Counter: 0 → VAL1 (14999) → reset
 *   PWM_A high while counter < VAL3
 *   So VAL3 = pulse width in counts
 *
 * Pin: P3_8, MUX Alt 5 = PWM0_A1
 *
 * FR coverage: F4.1, F4.2, F4.4
 *****************************************************************************/
#include "lock.h"
#include "lock_drv.h"
#include <MCXA153.h>

/* -------------------------------------------------------------------------
 * State
 * ---------------------------------------------------------------------- */
static bool g_is_open = false;

/* -------------------------------------------------------------------------
 * Init
 * ---------------------------------------------------------------------- */

void lock_drv_init(void)
{
    /* ---- Enable clocks ---- */

    /* eFlexPWM0 peripheral clock */
    MRCC0->MRCC_GLB_CC0_SET = MRCC_MRCC_GLB_CC0_FLEXPWM0(1);

    /* PORT3 clock (P3_8 is on PORT3) */
    MRCC0->MRCC_GLB_CC1_SET = MRCC_MRCC_GLB_CC1_PORT3(1);

    /* ---- Release from reset ---- */
    MRCC0->MRCC_GLB_RST0_SET = MRCC_MRCC_GLB_RST0_FLEXPWM0(1);
    MRCC0->MRCC_GLB_RST1_SET = MRCC_MRCC_GLB_RST1_PORT3(1);

    /* ---- Enable submodule 1 clock ---- */
    SYSCON->PWM0SUBCTL |= SYSCON_PWM0SUBCTL_CLK1_EN(1);

    /* ---- Pin mux: P3_8 = PWM0_A1 (Alt 5) ---- */
    PORT3->PCR[8] = PORT_PCR_LK(1) | PORT_PCR_MUX(5);

    /* ---- Configure submodule 1 ---- */

    /* Clear Load Okay for submodule 1 */
    FLEXPWM0->MCTRL |= PWM_MCTRL_CLDOK(0b010);

    /* Prescaler = 128 (PRSC = 0b111)
     * FULL = 1 (full-cycle reload)
     */
    FLEXPWM0->SM[1].CTRL = PWM_CTRL_PRSC(0b111) | PWM_CTRL_FULL(1);

    /* INDEP = 1 (PWM_A and PWM_B independent)
     * DBGEN = 1 (continue in debug mode)
     */
    FLEXPWM0->SM[1].CTRL2 = PWM_CTRL2_INDEP(1) | PWM_CTRL2_DBGEN(1);

    /* No polarity inversion — high pulse = active */
    FLEXPWM0->SM[1].OCTRL = 0;

    /* Disable fault handling for channel A */
    FLEXPWM0->SM[1].DISMAP[0] &= ~(PWM_DISMAP_DIS0A_MASK);

    /* Edge-aligned PWM:
     * INIT = 0 (counter starts at 0)
     * VAL1 = 14999 (period = 15000 counts = 20 ms at 750 kHz)
     * VAL2 = 0 (PWM_A rising edge at count 0)
     * VAL3 = pulse width (PWM_A falling edge)
     */
    FLEXPWM0->SM[1].INIT = 0;
    FLEXPWM0->SM[1].VAL0 = 0;
    FLEXPWM0->SM[1].VAL1 = LOCK_PWM_PERIOD - 1;  /* 14999 */
    FLEXPWM0->SM[1].VAL2 = 0;                     /* rising edge at 0 */
    FLEXPWM0->SM[1].VAL3 = LOCK_PULSE_CLOSED;     /* 750 = 1.0 ms = CLOSED */
    FLEXPWM0->SM[1].VAL4 = 0;
    FLEXPWM0->SM[1].VAL5 = 0;

    /* Load the values */
    FLEXPWM0->MCTRL |= PWM_MCTRL_LDOK(0b010);

    /* Enable PWM_A output for submodule 1 */
    FLEXPWM0->OUTEN |= PWM_OUTEN_PWMA_EN(0b010);

    /* Start the counter for submodule 1 */
    FLEXPWM0->MCTRL |= PWM_MCTRL_RUN(0b010);

    /* Initial state: CLOSED */
    g_is_open = false;
}

/* -------------------------------------------------------------------------
 * API
 * ---------------------------------------------------------------------- */

void lock_drv_open(void)
{
    lock_drv_set_pulse(LOCK_PULSE_OPEN);
    g_is_open = true;
}

void lock_drv_close(void)
{
    lock_drv_set_pulse(LOCK_PULSE_CLOSED);
    g_is_open = false;
}

void lock_drv_set_pulse(uint16_t pulse_counts)
{
    /* Clamp to valid servo range */
    if (pulse_counts < LOCK_PULSE_CLOSED) pulse_counts = LOCK_PULSE_CLOSED;
    if (pulse_counts > LOCK_PULSE_OPEN)   pulse_counts = LOCK_PULSE_OPEN;

    /* Clear Load Okay for submodule 1 */
    FLEXPWM0->MCTRL |= PWM_MCTRL_CLDOK(0b010);

    /* Update pulse width (VAL3 = falling edge position) */
    FLEXPWM0->SM[1].VAL3 = pulse_counts;

    /* Load new value */
    FLEXPWM0->MCTRL |= PWM_MCTRL_LDOK(0b010);
}

bool lock_drv_is_open(void)
{
    return g_is_open;
}
