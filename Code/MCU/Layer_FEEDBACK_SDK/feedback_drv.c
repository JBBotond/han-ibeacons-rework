/*! ***************************************************************************
 * \brief  feedback_drv.c — Layer 4 Feedback Driver Implementation
 * \file   feedback_drv.c
 *
 * Buzzer: CTIMER1, CLK_1M (1 MHz), prescaler 1, match channel 0 on P2_4.
 *   Frequency = 1 MHz / (MR3 + 1), duty = 50% → MR0 = MR3 / 2.
 *
 * LED: Direct GPIO3 register writes (same as leds.c, active LOW).
 *   P3_0  = blue
 *   P3_12 = red
 *   P3_13 = green
 *
 * Cue state machine: triggered by feedback_cue_*(), advanced by
 * feedback_drv_update() which checks elapsed ms via extern volatile ms.
 *****************************************************************************/
#include "feedback_drv.h"
#include <MCXA153.h>

/* -------------------------------------------------------------------------
 * Timestamp source (extern — provided by main.c SysTick)
 * ---------------------------------------------------------------------- */
extern volatile uint32_t ms;

/* -------------------------------------------------------------------------
 * Cue state machine
 * ---------------------------------------------------------------------- */
typedef enum
{
    FB_IDLE = 0,
    FB_PUZZLE,
    FB_UNLOCK,
    FB_ERROR,
    FB_FINAL
} fb_state_t;

static fb_state_t  g_state = FB_IDLE;
static uint32_t    g_start_ms = 0;
static uint8_t     g_final_step = 0;  /* sub-step for celebration */

/* -------------------------------------------------------------------------
 * Internal LED helpers (active LOW on GPIO3)
 * ---------------------------------------------------------------------- */
static inline void led_all_off(void)
{
    GPIO3->PSOR = (1 << 0) | (1 << 12) | (1 << 13);
}

static inline void led_red(void)
{
    GPIO3->PCOR = (1 << 12);
}

static inline void led_green(void)
{
    GPIO3->PCOR = (1 << 13);
}

static inline void led_blue(void)
{
    GPIO3->PCOR = (1 << 0);
}

/* -------------------------------------------------------------------------
 * Buzzer control via CTIMER1
 * ---------------------------------------------------------------------- */

void feedback_buzzer_set_freq(uint16_t freq_hz)
{
    if (freq_hz == 0)
    {
        feedback_buzzer_off();
        return;
    }

    /* Period in counts: 1 MHz / freq_hz */
    uint32_t period = 1000000UL / freq_hz;
    if (period < 2) period = 2;

    /* Stop timer briefly to update match registers */
    CTIMER1->TCR &= ~CTIMER_TCR_CEN_MASK;

    /* MR3 = period (sets PWM cycle length) */
    CTIMER1->MR[3] = period - 1;

    /* MR0 = 50% duty (match channel 0 output on P2_4) */
    CTIMER1->MR[0] = period / 2;

    /* Reset counter */
    CTIMER1->TC = 0;

    /* Re-enable */
    CTIMER1->TCR |= CTIMER_TCR_CEN(1);
}

void feedback_buzzer_off(void)
{
    /* Stop timer — output goes low */
    CTIMER1->TCR &= ~CTIMER_TCR_CEN_MASK;
    CTIMER1->TC = 0;

    /* Force output low via GPIO (pin stays in CTimer mux, but timer stopped = low) */
}

/* -------------------------------------------------------------------------
 * Init
 * ---------------------------------------------------------------------- */

void feedback_drv_init(void)
{
    /* ---- GPIO3 for RGB LED ---- */
    MRCC0->MRCC_GLB_CC1_SET = MRCC_MRCC_GLB_CC1_PORT3(1);
    MRCC0->MRCC_GLB_CC1_SET = MRCC_MRCC_GLB_CC1_GPIO3(1);
    MRCC0->MRCC_GLB_RST1_SET = MRCC_MRCC_GLB_RST1_PORT3(1);
    MRCC0->MRCC_GLB_RST1_SET = MRCC_MRCC_GLB_RST1_GPIO3(1);

    /* Configure P3_0, P3_12, P3_13 as GPIO output */
    PORT3->PCR[0]  = PORT_PCR_LK(1) | PORT_PCR_MUX(0);  /* GPIO alt 0 */
    PORT3->PCR[12] = PORT_PCR_LK(1) | PORT_PCR_MUX(0);
    PORT3->PCR[13] = PORT_PCR_LK(1) | PORT_PCR_MUX(0);

    /* All LEDs off (active low → set high) */
    GPIO3->PSOR = (1 << 0) | (1 << 12) | (1 << 13);
    /* Direction: output */
    GPIO3->PDDR |= (1 << 0) | (1 << 12) | (1 << 13);

    /* ---- CTIMER1 for buzzer PWM on P2_4 (CT1_MAT0) ---- */

    /* Clock source: CLK_1M (1 MHz) */
    MRCC0->MRCC_CTIMER1_CLKSEL = MRCC_MRCC_CTIMER1_CLKSEL_MUX(0b101);
    MRCC0->MRCC_CTIMER1_CLKDIV = 0;  /* DIV = 1 */

    /* Enable CTIMER1 function clock */
    SYSCON->CTIMERGLOBALSTARTEN |= SYSCON_CTIMERGLOBALSTARTEN_CTIMER1_CLK_EN(1);

    /* Enable peripheral clock and release from reset */
    MRCC0->MRCC_GLB_CC0_SET = MRCC_MRCC_GLB_CC0_CTIMER1(1);
    MRCC0->MRCC_GLB_RST0_SET = MRCC_MRCC_GLB_RST0_CTIMER1(1);

    /* Enable PORT2 clock for P2_4 pin mux */
    MRCC0->MRCC_GLB_CC0_SET = MRCC_MRCC_GLB_CC0_PORT2(1);
    MRCC0->MRCC_GLB_RST0_SET = MRCC_MRCC_GLB_RST0_PORT2(1);

    /* Configure P2_4 as CT1_MAT0 (Alt 4) */
    PORT2->PCR[4] = PORT_PCR_LK(1) | PORT_PCR_MUX(4);

    /* Prescaler = 0 (divide by 1) → count at 1 MHz */
    CTIMER1->PR = 0;

    /* Initial match values (silent — will be set by buzzer_set_freq) */
    CTIMER1->MR[0] = 0;
    CTIMER1->MR[3] = 999;  /* 1 kHz default period */

    /* MR3 resets the counter (PWM cycle) */
    CTIMER1->MCR = CTIMER_MCR_MR3R(1);

    /* Enable PWM mode for channels 0 and 3 */
    CTIMER1->PWMC = CTIMER_PWMC_PWMEN0(1) | CTIMER_PWMC_PWMEN3(1);

    /* Disable fault handling */
    /* (CTimer doesn't have DISMAP like eFlexPWM) */

    /* Don't start yet — buzzer starts silent */
    CTIMER1->TCR = 0;

    /* State machine idle */
    g_state = FB_IDLE;
}

/* -------------------------------------------------------------------------
 * Cue triggers
 * ---------------------------------------------------------------------- */

void feedback_cue_puzzle(void)
{
    g_state = FB_PUZZLE;
    g_start_ms = ms;
    led_all_off();
    led_green();
    /* No buzzer for puzzle cue — just LED */
}

void feedback_cue_unlock(void)
{
    g_state = FB_UNLOCK;
    g_start_ms = ms;
    led_all_off();
    led_blue();
    feedback_buzzer_set_freq(BUZZER_FREQ_HIGH);
}

void feedback_cue_error(void)
{
    g_state = FB_ERROR;
    g_start_ms = ms;
    led_all_off();
    led_red();
    feedback_buzzer_set_freq(BUZZER_FREQ_LOW);
}

void feedback_cue_final(void)
{
    g_state = FB_FINAL;
    g_start_ms = ms;
    g_final_step = 0;
    led_all_off();
    led_green();
    feedback_buzzer_set_freq(BUZZER_FREQ_MID);
}

/* -------------------------------------------------------------------------
 * State machine update (call from main loop)
 * ---------------------------------------------------------------------- */

bool feedback_drv_update(void)
{
    uint32_t elapsed = ms - g_start_ms;

    switch (g_state)
    {
    case FB_IDLE:
        return false;

    case FB_PUZZLE:
        if (elapsed >= CUE_PUZZLE_MS)
        {
            led_all_off();
            g_state = FB_IDLE;
        }
        break;

    case FB_UNLOCK:
        if (elapsed >= CUE_UNLOCK_MS)
        {
            led_all_off();
            feedback_buzzer_off();
            g_state = FB_IDLE;
        }
        break;

    case FB_ERROR:
        if (elapsed >= CUE_ERROR_MS)
        {
            led_all_off();
            feedback_buzzer_off();
            g_state = FB_IDLE;
        }
        break;

    case FB_FINAL:
    {
        /* Celebration: 6 steps × 500 ms = 3 s total */
        uint8_t step = (uint8_t)(elapsed / 500);
        if (step != g_final_step && step < 6)
        {
            g_final_step = step;
            led_all_off();
            switch (step % 3)
            {
            case 0: led_green(); break;
            case 1: led_blue();  break;
            case 2: led_red();   break;
            }
            /* Ascending tone: 1000, 1500, 2000, 2500, 3000, 3500 Hz */
            feedback_buzzer_set_freq(1000 + step * 500);
        }
        if (elapsed >= CUE_FINAL_MS)
        {
            led_all_off();
            feedback_buzzer_off();
            g_state = FB_IDLE;
        }
        break;
    }

    default:
        g_state = FB_IDLE;
        break;
    }

    return (g_state != FB_IDLE);
}

/* -------------------------------------------------------------------------
 * Utility
 * ---------------------------------------------------------------------- */

void feedback_drv_stop(void)
{
    led_all_off();
    feedback_buzzer_off();
    g_state = FB_IDLE;
}

bool feedback_drv_is_active(void)
{
    return (g_state != FB_IDLE);
}
