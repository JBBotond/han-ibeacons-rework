/*! ***************************************************************************
 * \brief  button_drv.c — Layer 4 Button Driver Implementation
 * \file   button_drv.c
 *
 * GPIO interrupt on falling edge + 20 ms software debounce.
 * Events are pushed into a small ring buffer from ISR context.
 *
 * Pins:
 *   SW2: P3_29 / GPIO3 pin 29 — active LOW
 *   SW3: P1_7  / GPIO1 pin 7  — active LOW
 *****************************************************************************/
#include "button_drv.h"
#include <MCXA153.h>
#include <string.h>

/* -------------------------------------------------------------------------
 * Timestamp source (extern — provided by main.c SysTick)
 * ---------------------------------------------------------------------- */
extern volatile uint32_t ms;

/* -------------------------------------------------------------------------
 * Event ring buffer
 * ---------------------------------------------------------------------- */
static button_event_t g_queue[BUTTON_EVENT_QUEUE];
static volatile uint8_t g_head = 0;
static volatile uint8_t g_tail = 0;

/* -------------------------------------------------------------------------
 * Debounce state
 * ---------------------------------------------------------------------- */
static volatile uint32_t g_last_sw2_ms = 0;
static volatile uint32_t g_last_sw3_ms = 0;

/* -------------------------------------------------------------------------
 * Lifetime counters
 * ---------------------------------------------------------------------- */
static volatile uint32_t g_count_sw2 = 0;
static volatile uint32_t g_count_sw3 = 0;

/* -------------------------------------------------------------------------
 * Internal: push event into ring (called from ISR)
 * ---------------------------------------------------------------------- */
static inline void push_event(button_id_t id, uint32_t ts)
{
    uint8_t next = (g_head + 1) % BUTTON_EVENT_QUEUE;
    if (next == g_tail) return;  /* queue full — drop oldest */

    g_queue[g_head].id = id;
    g_queue[g_head].timestamp = ts;
    g_head = next;
}

/* -------------------------------------------------------------------------
 * Init
 * ---------------------------------------------------------------------- */

void button_drv_init(void)
{
    /* ---- Enable clocks ---- */
    /* PORT1 + GPIO1 (for SW3 on P1_7) */
    MRCC0->MRCC_GLB_CC0_SET = MRCC_MRCC_GLB_CC0_PORT1(1);
    MRCC0->MRCC_GLB_CC1_SET = MRCC_MRCC_GLB_CC1_GPIO1(1);

    /* PORT3 + GPIO3 (for SW2 on P3_29) */
    MRCC0->MRCC_GLB_CC1_SET = MRCC_MRCC_GLB_CC1_PORT3(1);
    MRCC0->MRCC_GLB_CC1_SET = MRCC_MRCC_GLB_CC1_GPIO3(1);

    /* ---- Release from reset ---- */
    MRCC0->MRCC_GLB_RST0_SET = MRCC_MRCC_GLB_RST0_PORT1(1);
    MRCC0->MRCC_GLB_RST1_SET = MRCC_MRCC_GLB_RST1_GPIO1(1);
    MRCC0->MRCC_GLB_RST1_SET = MRCC_MRCC_GLB_RST1_PORT3(1);
    MRCC0->MRCC_GLB_RST1_SET = MRCC_MRCC_GLB_RST1_GPIO3(1);

    /* ---- Pin mux: GPIO input with input buffer enabled ---- */
    /* P1_7 (SW3): MUX=0 (GPIO), IBE=1 (input buffer enable) */
    PORT1->PCR[7] = PORT_PCR_LK(1) | PORT_PCR_IBE(1) | PORT_PCR_MUX(0);

    /* P3_29 (SW2): MUX=0 (GPIO), IBE=1 (input buffer enable) */
    PORT3->PCR[29] = PORT_PCR_LK(1) | PORT_PCR_IBE(1) | PORT_PCR_MUX(0);

    /* ---- Direction: input (default, but explicit) ---- */
    GPIO1->PDDR &= ~(1UL << 7);
    GPIO3->PDDR &= ~(1UL << 29);

    /* ---- Interrupt: falling edge ---- */
    /* Clear flag + set IRQC = 1010 (ISF flag + interrupt on falling edge) */
    GPIO1->ICR[7]  = GPIO_ICR_ISF(1) | GPIO_ICR_IRQC(0b1010);
    GPIO3->ICR[29] = GPIO_ICR_ISF(1) | GPIO_ICR_IRQC(0b1010);

    /* ---- NVIC ---- */
    NVIC_SetPriority(GPIO1_IRQn, 2);  /* Higher priority than UART (3) */
    NVIC_ClearPendingIRQ(GPIO1_IRQn);
    NVIC_EnableIRQ(GPIO1_IRQn);

    NVIC_SetPriority(GPIO3_IRQn, 2);
    NVIC_ClearPendingIRQ(GPIO3_IRQn);
    NVIC_EnableIRQ(GPIO3_IRQn);

    /* ---- Enable global interrupts ---- */
    __enable_irq();

    /* ---- Clear state ---- */
    g_head = 0;
    g_tail = 0;
    g_last_sw2_ms = 0;
    g_last_sw3_ms = 0;
    g_count_sw2 = 0;
    g_count_sw3 = 0;
}

/* -------------------------------------------------------------------------
 * ISR handlers
 * ---------------------------------------------------------------------- */

void GPIO1_IRQHandler(void)
{
    NVIC_ClearPendingIRQ(GPIO1_IRQn);

    /* P1_7 (SW3) triggered? */
    if ((GPIO1->ISFR[0] & GPIO_ISFR_ISF7(1)) != 0)
    {
        GPIO1->ISFR[0] = GPIO_ISFR_ISF7(1);  /* Clear flag */

        /* Debounce: ignore if < 20 ms since last valid press */
        uint32_t now = ms;
        if ((now - g_last_sw3_ms) >= BUTTON_DEBOUNCE_MS)
        {
            g_last_sw3_ms = now;
            g_count_sw3++;
            push_event(BTN_SW3, now);
        }
    }
}

void GPIO3_IRQHandler(void)
{
    NVIC_ClearPendingIRQ(GPIO3_IRQn);

    /* P3_29 (SW2) triggered? */
    if ((GPIO3->ISFR[0] & GPIO_ISFR_ISF29(1)) != 0)
    {
        GPIO3->ISFR[0] = GPIO_ISFR_ISF29(1);  /* Clear flag */

        /* Debounce: ignore if < 20 ms since last valid press */
        uint32_t now = ms;
        if ((now - g_last_sw2_ms) >= BUTTON_DEBOUNCE_MS)
        {
            g_last_sw2_ms = now;
            g_count_sw2++;
            push_event(BTN_SW2, now);
        }
    }
}

/* -------------------------------------------------------------------------
 * API
 * ---------------------------------------------------------------------- */

bool button_drv_get_event(button_event_t *evt)
{
    if (evt == NULL) return false;
    if (g_tail == g_head) return false;  /* empty */

    *evt = g_queue[g_tail];
    g_tail = (g_tail + 1) % BUTTON_EVENT_QUEUE;
    return true;
}

uint8_t button_drv_pending(void)
{
    return (uint8_t)((g_head - g_tail + BUTTON_EVENT_QUEUE) % BUTTON_EVENT_QUEUE);
}

void button_drv_flush(void)
{
    g_tail = g_head;
}

uint32_t button_drv_get_count(button_id_t id)
{
    if (id == BTN_SW2) return g_count_sw2;
    if (id == BTN_SW3) return g_count_sw3;
    return 0;
}
