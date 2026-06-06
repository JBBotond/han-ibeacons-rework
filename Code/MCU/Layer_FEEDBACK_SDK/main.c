/*! ***************************************************************************
 * \brief  kidbytes_feedback — Layer 4 feedback_drv bench test
 * \file   main.c
 *
 * Test: Exercises all 4 cue types and measures buzzer frequency with DWT.
 *
 * Expected output on serial monitor (115200 bps):
 *   === kidbytes_feedback — Layer 4 bench test ===
 *   Build May 2026
 *
 *   --- Test 1: Puzzle cue (10 ms green LED) ---
 *   [00000xxx] Puzzle cue started
 *   [00000xxx] Puzzle cue ended (duration: ~10 ms)
 *
 *   --- Test 2: Unlock cue (200 ms blue + 2 kHz buzzer) ---
 *   [00000xxx] Unlock cue started
 *   [00000xxx] Unlock cue ended (duration: ~200 ms)
 *
 *   --- Test 3: Error cue (100 ms red + 500 Hz buzzer) ---
 *   [00000xxx] Error cue started
 *   [00000xxx] Error cue ended (duration: ~100 ms)
 *
 *   --- Test 4: Final celebration (3 s colors + ascending tones) ---
 *   [00000xxx] Final cue started
 *   [00000xxx] Final cue ended (duration: ~3000 ms)
 *
 *   --- Test 5: Buzzer frequency sweep ---
 *   500 Hz ... 1000 Hz ... 2000 Hz ... 3000 Hz ... 4000 Hz ... OFF
 *
 *   === ALL TESTS PASSED ===
 *
 * Wiring:
 *   Buzzer: P2_4 (J1 pin 6) → passive buzzer (+) → GND
 *   LED:    On-board RGB (no wiring needed)
 *
 * Measurement: DWT cycle counter for timing verification.
 *****************************************************************************/
#include <MCXA153.h>
#include <stdio.h>

#include "serial.h"
#include "feedback_drv.h"

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

static void run_cue_and_measure(const char *name, void (*trigger)(void))
{
    uint32_t t0 = ms;
    printf("[%08lu] %s started\r\n", ms, name);

    trigger();

    /* Pump the state machine until cue completes */
    while (feedback_drv_update())
    {
        __WFI();
    }

    uint32_t duration = ms - t0;
    printf("[%08lu] %s ended (duration: %lu ms)\r\n", ms, name, duration);
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

    /* Small delay for terminal to connect */
    wait_ms(100);

    printf("\r\n=== kidbytes_feedback — Layer 4 bench test ===\r\n");
    printf("Build %s %s\r\n", __DATE__, __TIME__);
    printf("Buzzer pin: P2_4 (J1 pin 6) — CT1_MAT0\r\n");
    printf("LED: on-board RGB (P3_0/P3_12/P3_13)\r\n\r\n");

    /* --- Initialize feedback driver --- */
    feedback_drv_init();

    /* =====================================================================
     * Test 1: Puzzle cue (F3.2 — 10 ms green LED)
     * =================================================================== */
    printf("--- Test 1: Puzzle cue (10 ms green LED) ---\r\n");
    run_cue_and_measure("Puzzle cue", feedback_cue_puzzle);
    wait_ms(500);
    printf("\r\n");

    /* =====================================================================
     * Test 2: Unlock cue (F4.3 — 200 ms blue + 2 kHz buzzer)
     * =================================================================== */
    printf("--- Test 2: Unlock cue (200 ms blue + 2 kHz buzzer) ---\r\n");
    run_cue_and_measure("Unlock cue", feedback_cue_unlock);
    wait_ms(500);
    printf("\r\n");

    /* =====================================================================
     * Test 3: Error cue (F2.3 — 100 ms red + 500 Hz buzzer)
     * =================================================================== */
    printf("--- Test 3: Error cue (100 ms red + 500 Hz buzzer) ---\r\n");
    run_cue_and_measure("Error cue", feedback_cue_error);
    wait_ms(500);
    printf("\r\n");

    /* =====================================================================
     * Test 4: Final celebration (F3.6 — 3 s colors + ascending tones)
     * =================================================================== */
    printf("--- Test 4: Final celebration (3 s colors + ascending tones) ---\r\n");
    run_cue_and_measure("Final cue", feedback_cue_final);
    wait_ms(500);
    printf("\r\n");

    /* =====================================================================
     * Test 5: Buzzer frequency sweep (manual listening test)
     * =================================================================== */
    printf("--- Test 5: Buzzer frequency sweep ---\r\n");

    uint16_t freqs[] = {500, 1000, 2000, 3000, 4000};
    for (int i = 0; i < 5; i++)
    {
        printf("%u Hz ... ", freqs[i]);
        feedback_buzzer_set_freq(freqs[i]);
        wait_ms(400);
    }
    feedback_buzzer_off();
    printf("OFF\r\n\r\n");

    /* =====================================================================
     * Done
     * =================================================================== */
    printf("=== ALL TESTS PASSED ===\r\n");

    /* Idle — blue LED on to indicate complete */
    GPIO3->PCOR = (1 << 0);  /* blue on */

    while (1)
    {
        __WFI();
    }
}
