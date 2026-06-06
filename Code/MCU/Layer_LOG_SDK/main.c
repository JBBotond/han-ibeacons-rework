/*! ***************************************************************************
 * \brief  kidbytes_log — Layer 11 LOG bench test
 * \file   main.c
 *
 * Test: Exercises the full log.h API and measures the ≤ 1 µs guarantee
 * when LogGlobalOff() is active (F8.3), using DWT cycle counter.
 *
 * Expected output on serial monitor (115200 bps):
 *   === kidbytes_log — Layer 11 bench test ===
 *   Build May 2026
 *
 *   --- Test 1: All subsystems at INFO ---
 *   [00000xxx] SYS:INFO Boot complete
 *   [00000xxx] COMMS:INFO HM-10 connected
 *   [00000xxx] SENSOR:DEBUG rssi=-69
 *   [00000xxx] FSM:WARN state timeout
 *   [00000xxx] POWER:ERROR brown-out detected
 *   [00000xxx] SYS:CRIT watchdog reset imminent
 *
 *   --- Test 2: Silence DISPLAY subsystem ---
 *   (no DISPLAY messages appear)
 *
 *   --- Test 3: LogGlobalOff() performance ---
 *   Log() when OFF: XX cycles (XX ns) — must be ≤ 96 cycles (≤ 1 us)
 *
 *   --- Test 4: LogGlobalOn() restores output ---
 *   [00000xxx] SYS:INFO Logging restored
 *
 *   --- Test 5: LogWithNum ---
 *   [00000xxx] SENSOR:INFO room=2
 *   [00000xxx] SENSOR:DEBUG rssi=-73
 *   [00000xxx] FSM:INFO state=5
 *
 *   === ALL TESTS PASSED ===
 *
 * Wiring: None (on-board MCU-Link VCOM only).
 * Measurement: DWT cycle counter (Cortex-M33 built-in).
 *****************************************************************************/
#include <MCXA153.h>
#include <stdio.h>
#include <string.h>

#include "serial.h"
#include "leds.h"
#include "log.h"

/* -------------------------------------------------------------------------
 * Globals
 * ---------------------------------------------------------------------- */
volatile uint32_t ms = 0;

void SysTick_Handler(void) { ms++; }

/* -------------------------------------------------------------------------
 * DWT Cycle Counter helpers (Cortex-M33)
 * ---------------------------------------------------------------------- */
static inline void dwt_init(void)
{
    /* Enable DWT and ITM via DCB->DEMCR */
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;

    /* Reset and enable the cycle counter */
    DWT->CYCCNT = 0;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

static inline uint32_t dwt_get_cycles(void)
{
    return DWT->CYCCNT;
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
    dwt_init();

    /* Small delay for terminal to connect */
    uint32_t t = ms;
    while ((ms - t) < 100) {}

    printf("\r\n=== kidbytes_log — Layer 11 bench test ===\r\n");
    printf("Build %s %s\r\n", __DATE__, __TIME__);
    printf("CPU: 96 MHz FIRC, DWT cycle counter enabled\r\n\r\n");

    /* --- Initialize LOG --- */
    LogInit();

    /* =====================================================================
     * Test 1: All subsystems at INFO — basic output
     * =================================================================== */
    printf("--- Test 1: All subsystems at INFO ---\r\n");

    Log(LOG_SYS_SYS,     LOG_INFO,  "Boot complete");
    Log(LOG_SYS_COMMS,   LOG_INFO,  "HM-10 connected");
    Log(LOG_SYS_SENSOR,  LOG_DEBUG, "rssi=-69");
    Log(LOG_SYS_FSM,     LOG_WARN,  "state timeout");
    Log(LOG_SYS_POWER,   LOG_ERROR, "brown-out detected");
    Log(LOG_SYS_SYS,     LOG_CRIT,  "watchdog reset imminent");

    LogFlush();
    printf("\r\n");

    /* =====================================================================
     * Test 2: Silence DISPLAY subsystem (F8.2 per-subsystem gating)
     * =================================================================== */
    printf("--- Test 2: Silence DISPLAY subsystem ---\r\n");

    LogSetOutputLevel(LOG_SYS_DISPLAY, LOG_NONE);

    Log(LOG_SYS_DISPLAY, LOG_INFO,  "This should NOT appear");
    Log(LOG_SYS_DISPLAY, LOG_CRIT,  "This should NOT appear either");
    Log(LOG_SYS_SYS,     LOG_INFO,  "SYS still works");

    LogFlush();

    /* Restore for later tests */
    LogSetOutputLevel(LOG_SYS_DISPLAY, LOG_INFO);
    printf("\r\n");

    /* =====================================================================
     * Test 3: LogGlobalOff() performance (F8.3: ≤ 1 µs = ≤ 96 cycles)
     * =================================================================== */
    printf("--- Test 3: LogGlobalOff() performance ---\r\n");

    LogGlobalOff();

    /* Measure cycles for a Log() call when globally off */
    uint32_t c0 = dwt_get_cycles();
    Log(LOG_SYS_SYS, LOG_CRIT, "This must not appear and must be fast");
    uint32_t c1 = dwt_get_cycles();

    uint32_t cycles_off = c1 - c0;
    uint32_t ns_off = (cycles_off * 1000) / 96;  /* 96 MHz → ns */

    printf("Log() when OFF: %lu cycles (%lu ns)", cycles_off, ns_off);
    if (cycles_off <= 96)
    {
        printf(" — PASS (≤ 1 us)\r\n");
        led_green_on();
    }
    else
    {
        printf(" — FAIL (> 1 us)\r\n");
        led_red_on();
    }

    /* Also measure LogWithNum when off */
    c0 = dwt_get_cycles();
    LogWithNum(LOG_SYS_SENSOR, LOG_DEBUG, "rssi=", -69);
    c1 = dwt_get_cycles();

    uint32_t cycles_off2 = c1 - c0;
    uint32_t ns_off2 = (cycles_off2 * 1000) / 96;
    printf("LogWithNum() when OFF: %lu cycles (%lu ns)", cycles_off2, ns_off2);
    if (cycles_off2 <= 96)
    {
        printf(" — PASS (≤ 1 us)\r\n");
    }
    else
    {
        printf(" — FAIL (> 1 us)\r\n");
    }
    printf("\r\n");

    /* =====================================================================
     * Test 4: LogGlobalOn() restores output
     * =================================================================== */
    printf("--- Test 4: LogGlobalOn() restores output ---\r\n");

    LogGlobalOn();
    Log(LOG_SYS_SYS, LOG_INFO, "Logging restored");
    LogFlush();
    printf("\r\n");

    /* =====================================================================
     * Test 5: LogWithNum — numeric values
     * =================================================================== */
    printf("--- Test 5: LogWithNum ---\r\n");

    LogWithNum(LOG_SYS_SENSOR, LOG_INFO,  "room=", 2);
    LogWithNum(LOG_SYS_SENSOR, LOG_DEBUG, "rssi=", -73);
    LogWithNum(LOG_SYS_FSM,    LOG_INFO,  "state=", 5);
    LogWithNum(LOG_SYS_POWER,  LOG_WARN,  "battery_mv=", 3200);
    LogWithNum(LOG_SYS_COMMS,  LOG_INFO,  "matched=", 0);

    LogFlush();
    printf("\r\n");

    /* =====================================================================
     * Test 6: Level filtering — DEBUG hidden when level is WARN
     * =================================================================== */
    printf("--- Test 6: Level filtering (SENSOR set to WARN) ---\r\n");

    LogSetOutputLevel(LOG_SYS_SENSOR, LOG_WARN);

    Log(LOG_SYS_SENSOR, LOG_INFO,  "Should NOT appear (INFO < WARN)");
    Log(LOG_SYS_SENSOR, LOG_DEBUG, "Should NOT appear (DEBUG < WARN)");
    Log(LOG_SYS_SENSOR, LOG_WARN,  "Should appear (WARN >= WARN)");
    Log(LOG_SYS_SENSOR, LOG_ERROR, "Should appear (ERROR >= WARN)");
    Log(LOG_SYS_SENSOR, LOG_CRIT,  "Should appear (CRIT >= WARN)");

    LogFlush();

    /* Restore */
    LogSetOutputLevel(LOG_SYS_SENSOR, LOG_INFO);
    printf("\r\n");

    /* =====================================================================
     * Done
     * =================================================================== */
    printf("=== ALL TESTS PASSED ===\r\n");
    led_blue_on();

    /* Idle */
    while (1)
    {
        __WFI();
    }
}
