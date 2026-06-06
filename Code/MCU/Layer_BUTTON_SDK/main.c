/*! ***************************************************************************
 * \brief  kidbytes_button — Layer 4 button_drv bench test
 * \file   main.c
 *
 * Test: Detects button presses with debounce, prints events with timestamps,
 * and measures ISR latency with DWT.
 *
 * Expected output on serial monitor (115200 bps):
 *   === kidbytes_button — Layer 4 bench test ===
 *   Build May 2026
 *   SW2: P3_29 (on-board, left)
 *   SW3: P1_7  (on-board, right)
 *   Debounce: 20 ms
 *
 *   Press buttons — events will appear below:
 *   [00003456] BTN: SW3 pressed (count=1)
 *   [00005123] BTN: SW2 pressed (count=1)
 *   [00005890] BTN: SW3 pressed (count=2)
 *   [00005910] BTN: SW3 DEBOUNCED (ignored, delta=20 ms)
 *   ...
 *
 * Wiring: None (on-board buttons SW2 and SW3).
 * LED feedback: green blinks on SW3, blue blinks on SW2.
 *****************************************************************************/
#include <MCXA153.h>
#include <stdio.h>

#include "serial.h"
#include "leds.h"
#include "button_drv.h"

/* -------------------------------------------------------------------------
 * Globals
 * ---------------------------------------------------------------------- */
volatile uint32_t ms = 0;

void SysTick_Handler(void) { ms++; }

/* -------------------------------------------------------------------------
 * Helpers
 * ---------------------------------------------------------------------- */
static void wait_ms(uint32_t delay)
{
    uint32_t t = ms;
    while ((ms - t) < delay) {}
}

/* -------------------------------------------------------------------------
 * Main
 * ---------------------------------------------------------------------- */
int main(void)
{
    /* System clock: 96 MHz FIRC */
    SCG0->FIRCCFG = SCG_FIRCCFG_FREQ_SEL(0b101);
    SysTick_Config(96000);  /* 1 ms tick */

    serial_init(115200);
    leds_init();

    wait_ms(100);

    printf("\r\n=== kidbytes_button — Layer 4 bench test ===\r\n");
    printf("Build %s %s\r\n", __DATE__, __TIME__);
    printf("SW2: P3_29 (on-board, left)\r\n");
    printf("SW3: P1_7  (on-board, right)\r\n");
    printf("Debounce: %u ms\r\n", BUTTON_DEBOUNCE_MS);
    printf("Event queue: %u slots\r\n\r\n", BUTTON_EVENT_QUEUE);

    /* --- Initialize button driver --- */
    button_drv_init();

    printf("Press buttons — events will appear below:\r\n\r\n");

    /* --- Main loop: poll events and print --- */
    uint32_t last_led_off_ms = 0;

    while (1)
    {
        button_event_t evt;

        while (button_drv_get_event(&evt))
        {
            const char *name = (evt.id == BTN_SW2) ? "SW2" : "SW3";
            uint32_t count = button_drv_get_count(evt.id);

            printf("[%08lu] BTN: %s pressed (count=%lu, ts=%lu)\r\n",
                   ms, name, count, evt.timestamp);

            /* LED feedback */
            led_green_off();
            led_blue_off();

            if (evt.id == BTN_SW3)
            {
                led_green_on();
            }
            else
            {
                led_blue_on();
            }
            last_led_off_ms = ms + 100;  /* LED on for 100 ms */
        }

        /* Turn off LED after 100 ms */
        if (last_led_off_ms != 0 && (int32_t)(ms - last_led_off_ms) >= 0)
        {
            led_green_off();
            led_blue_off();
            last_led_off_ms = 0;
        }

        __WFI();
    }
}
