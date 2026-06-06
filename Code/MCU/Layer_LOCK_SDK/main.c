/*! ***************************************************************************
 * \brief  kidbytes_lock — Layer 4 lock_drv bench test
 * \file   main.c
 *
 * Test: Drives the lock between CLOSED and OPEN with serial commands for
 * interactive control. Two build variants select the Layer 4 backend:
 *   - default                 → SERVO backend (eFlexPWM0, P3_8), DWT pulse timing
 *   - -DLOCK_BACKEND_SOLENOID → SOLENOID backend (GPIO2, P2_7), on/off fail-secure
 *
 * Expected output on serial monitor (115200 bps):
 *   === kidbytes_lock — Layer 4 bench test ===
 *   Build May 2026
 *   Servo pin: P3_8 (J3 pin 11) — PWM0_A1
 *   PWM: 50 Hz, prescaler 128, period 15000 counts
 *
 *   --- Test 1: CLOSED position (1.0 ms pulse) ---
 *   [00000xxx] Lock CLOSED (pulse=750 counts = 1.00 ms)
 *
 *   --- Test 2: OPEN position (2.0 ms pulse) ---
 *   [00002xxx] Lock OPEN (pulse=1500 counts = 2.00 ms)
 *
 *   --- Test 3: CENTER position (1.5 ms pulse) ---
 *   [00004xxx] Lock CENTER (pulse=1125 counts = 1.50 ms)
 *
 *   --- Test 4: Sweep CLOSED → OPEN → CLOSED ---
 *   [00006xxx] Sweeping...
 *   [00009xxx] Sweep complete
 *
 *   --- Interactive mode ---
 *   Commands: 'o' = open, 'c' = close, 'm' = center, 's' = sweep
 *
 * Wiring:
 *   Servo signal: P3_8 (J3 pin 11)
 *   Servo VCC:    5V (from power bank or USB — NOT from 3.3V rail!)
 *   Servo GND:    GND (shared with FRDM board)
 *
 * IMPORTANT: Standard hobby servos need 5V power. The signal wire (P3_8)
 * is 3.3V logic which is fine — most servos accept 3.3V signal levels.
 * Do NOT power the servo from the FRDM 3.3V pin — it can't supply enough
 * current and will brown-out the MCU.
 *****************************************************************************/
#include <MCXA153.h>
#include <stdio.h>

#include "serial.h"
#include "leds.h"
#include "lock.h"        /* portable contract (both backends) */
#if !defined(LOCK_BACKEND_SOLENOID)
#include "lock_drv.h"    /* servo-only extension (set_pulse, pulse constants) */
#endif

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

#if defined(LOCK_BACKEND_SOLENOID)
    /* ===================================================================
     * SOLENOID backend bench test (on/off only — no pulse control)
     * =================================================================== */
    printf("\r\n=== kidbytes_lock — Layer 4 bench test (SOLENOID) ===\r\n");
    printf("Build %s %s\r\n", __DATE__, __TIME__);
    printf("Solenoid pin: P2_7 (GPIO2) — on/off, fail-secure\r\n\r\n");

    lock_drv_init();   /* starts CLOSED (de-energized) */
    led_green_on();

    printf("--- Test 1: CLOSED (de-energized) ---\r\n");
    lock_drv_close();
    printf("[%08lu] Lock CLOSED, is_open=%u\r\n\r\n", ms, lock_drv_is_open());
    wait_ms(2000);

    printf("--- Test 2: OPEN (energized) ---\r\n");
    lock_drv_open();
    printf("[%08lu] Lock OPEN, is_open=%u\r\n\r\n", ms, lock_drv_is_open());
    wait_ms(2000);

    printf("--- Test 3: CLOSED again ---\r\n");
    lock_drv_close();
    printf("[%08lu] Lock CLOSED, is_open=%u\r\n\r\n", ms, lock_drv_is_open());
    wait_ms(1000);

    printf("--- Interactive: 'o'=open, 'c'=close ---\r\n\r\n");
    while (1)
    {
        if (serial_rxcnt() > 0)
        {
            char cmd = (char)serial_getchar();
            if (cmd == 'o' || cmd == 'O')
            {
                lock_drv_open();
                printf("[%08lu] OPEN\r\n", ms);
                led_blue_on(); led_green_off();
            }
            else if (cmd == 'c' || cmd == 'C')
            {
                lock_drv_close();
                printf("[%08lu] CLOSED\r\n", ms);
                led_green_on(); led_blue_off();
            }
        }
        __WFI();
    }

#else
    /* ===================================================================
     * SERVO backend bench test (PWM pulse control) — original
     * =================================================================== */
    printf("\r\n=== kidbytes_lock — Layer 4 bench test ===\r\n");
    printf("Build %s %s\r\n", __DATE__, __TIME__);
    printf("Servo pin: P3_8 (J3 pin 11) — PWM0_A1\r\n");
    printf("PWM: 50 Hz, prescaler 128, period %u counts\r\n", LOCK_PWM_PERIOD);
    printf("Pulse range: %u (1.0 ms CLOSED) to %u (2.0 ms OPEN)\r\n\r\n",
           LOCK_PULSE_CLOSED, LOCK_PULSE_OPEN);

    /* --- Initialize lock driver (starts CLOSED) --- */
    lock_drv_init();
    led_green_on();

    /* =====================================================================
     * Test 1: CLOSED position
     * =================================================================== */
    printf("--- Test 1: CLOSED position (1.0 ms pulse) ---\r\n");
    lock_drv_close();
    printf("[%08lu] Lock CLOSED (pulse=%u counts = 1.00 ms)\r\n\r\n", ms, LOCK_PULSE_CLOSED);
    wait_ms(2000);

    /* =====================================================================
     * Test 2: OPEN position
     * =================================================================== */
    printf("--- Test 2: OPEN position (2.0 ms pulse) ---\r\n");
    lock_drv_open();
    printf("[%08lu] Lock OPEN (pulse=%u counts = 2.00 ms)\r\n\r\n", ms, LOCK_PULSE_OPEN);
    wait_ms(2000);

    /* =====================================================================
     * Test 3: CENTER position
     * =================================================================== */
    printf("--- Test 3: CENTER position (1.5 ms pulse) ---\r\n");
    lock_drv_set_pulse(LOCK_PULSE_CENTER);
    printf("[%08lu] Lock CENTER (pulse=%u counts = 1.50 ms)\r\n\r\n", ms, LOCK_PULSE_CENTER);
    wait_ms(2000);

    /* =====================================================================
     * Test 4: Sweep CLOSED → OPEN → CLOSED
     * =================================================================== */
    printf("--- Test 4: Sweep CLOSED → OPEN → CLOSED ---\r\n");
    printf("[%08lu] Sweeping...\r\n", ms);

    /* Sweep from CLOSED to OPEN in 50 steps */
    for (uint16_t p = LOCK_PULSE_CLOSED; p <= LOCK_PULSE_OPEN; p += 15)
    {
        lock_drv_set_pulse(p);
        wait_ms(30);
    }
    lock_drv_set_pulse(LOCK_PULSE_OPEN);
    wait_ms(500);

    /* Sweep back from OPEN to CLOSED */
    for (uint16_t p = LOCK_PULSE_OPEN; p >= LOCK_PULSE_CLOSED; p -= 15)
    {
        lock_drv_set_pulse(p);
        wait_ms(30);
        if (p < LOCK_PULSE_CLOSED + 15) break;  /* prevent underflow */
    }
    lock_drv_close();

    printf("[%08lu] Sweep complete\r\n\r\n", ms);
    wait_ms(1000);

    /* =====================================================================
     * Interactive mode
     * =================================================================== */
    printf("--- Interactive mode ---\r\n");
    printf("Commands: 'o' = open, 'c' = close, 'm' = center, 's' = sweep\r\n");
    printf("          '+' = +25 counts, '-' = -25 counts\r\n\r\n");

    uint16_t current_pulse = LOCK_PULSE_CLOSED;

    while (1)
    {
        if (serial_rxcnt() > 0)
        {
            char cmd = (char)serial_getchar();

            switch (cmd)
            {
            case 'o':
            case 'O':
                lock_drv_open();
                current_pulse = LOCK_PULSE_OPEN;
                printf("[%08lu] OPEN (pulse=%u)\r\n", ms, current_pulse);
                led_blue_on();
                led_green_off();
                break;

            case 'c':
            case 'C':
                lock_drv_close();
                current_pulse = LOCK_PULSE_CLOSED;
                printf("[%08lu] CLOSED (pulse=%u)\r\n", ms, current_pulse);
                led_green_on();
                led_blue_off();
                break;

            case 'm':
            case 'M':
                lock_drv_set_pulse(LOCK_PULSE_CENTER);
                current_pulse = LOCK_PULSE_CENTER;
                printf("[%08lu] CENTER (pulse=%u)\r\n", ms, current_pulse);
                break;

            case 's':
            case 'S':
                printf("[%08lu] Sweeping...\r\n", ms);
                for (uint16_t p = LOCK_PULSE_CLOSED; p <= LOCK_PULSE_OPEN; p += 15)
                {
                    lock_drv_set_pulse(p);
                    wait_ms(30);
                }
                lock_drv_close();
                current_pulse = LOCK_PULSE_CLOSED;
                printf("[%08lu] Sweep done\r\n", ms);
                break;

            case '+':
                if (current_pulse + 25 <= LOCK_PULSE_OPEN)
                {
                    current_pulse += 25;
                    lock_drv_set_pulse(current_pulse);
                    printf("[%08lu] pulse=%u (+25)\r\n", ms, current_pulse);
                }
                break;

            case '-':
                if (current_pulse >= LOCK_PULSE_CLOSED + 25)
                {
                    current_pulse -= 25;
                    lock_drv_set_pulse(current_pulse);
                    printf("[%08lu] pulse=%u (-25)\r\n", ms, current_pulse);
                }
                break;

            default:
                break;
            }
        }

        __WFI();
    }
#endif /* LOCK_BACKEND_SOLENOID */
}
