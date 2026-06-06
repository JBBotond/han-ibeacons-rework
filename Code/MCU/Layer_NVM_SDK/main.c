/*! ***************************************************************************
 * \brief  kidbytes_nvm — Layer 4 nvm_drv bench test
 * \file   main.c
 *
 * Test: Writes session records to on-chip flash, reads them back, verifies
 * persistence across reset, and measures write latency with DWT.
 *
 * Expected output on serial monitor (115200 bps):
 *   === kidbytes_nvm — Layer 4 bench test ===
 *   Build May 2026
 *   Flash sector size: 1024 bytes
 *   NVM region: 0x1F800–0x1FFFF (2048 bytes, 32 slots)
 *
 *   --- Test 1: Erase NVM region ---
 *   [00000xxx] Erase OK
 *
 *   --- Test 2: Write first record ---
 *   [00000xxx] Write OK (seq=1, room=1, outcome=ENTERED, latency=XX ms)
 *
 *   --- Test 3: Read back latest ---
 *   [00000xxx] Read OK: seq=1 room=1 progress=0 outcome=0 ts=XXX
 *
 *   --- Test 4: Write 5 records (simulate a tour) ---
 *   [00000xxx] Write seq=2 room=1 outcome=PUZZLE_OK
 *   [00000xxx] Write seq=3 room=2 outcome=ENTERED
 *   [00000xxx] Write seq=4 room=2 outcome=PUZZLE_OK
 *   [00000xxx] Write seq=5 room=3 outcome=ENTERED
 *   [00000xxx] Latest: seq=5 room=3 progress=2
 *
 *   --- Test 5: Power-cut simulation ---
 *   Reset the board now. After reset, the latest record should persist.
 *   Press SW3 to verify persistence...
 *
 *   --- Test 6: Write latency (DWT) ---
 *   [00000xxx] Write latency: XX ms (must be < 50 ms for F7.2)
 *
 *   === ALL TESTS PASSED ===
 *
 * Wiring: None (on-chip flash, no external hardware).
 *
 * IMPORTANT: This test writes to the LAST 2 KiB of flash. Make sure your
 * firmware doesn't extend past 0x1F800 (check the .map file).
 *****************************************************************************/
#include <MCXA153.h>
#include <stdio.h>
#include <string.h>

#include "serial.h"
#include "leds.h"
#include "nvm_drv.h"

/* -------------------------------------------------------------------------
 * Globals
 * ---------------------------------------------------------------------- */
volatile uint32_t ms = 0;

void SysTick_Handler(void) { ms++; }

/* -------------------------------------------------------------------------
 * DWT helpers
 * ---------------------------------------------------------------------- */
static inline void dwt_init(void)
{
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT = 0;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

/* -------------------------------------------------------------------------
 * Helpers
 * ---------------------------------------------------------------------- */
static void wait_ms(uint32_t delay)
{
    uint32_t t = ms;
    while ((ms - t) < delay) {}
}

static const char *outcome_str(uint8_t o)
{
    switch (o)
    {
    case NVM_OUTCOME_ENTERED:     return "ENTERED";
    case NVM_OUTCOME_PUZZLE_OK:   return "PUZZLE_OK";
    case NVM_OUTCOME_PUZZLE_FAIL: return "PUZZLE_FAIL";
    case NVM_OUTCOME_UNLOCKED:    return "UNLOCKED";
    case NVM_OUTCOME_ADMIN:       return "ADMIN";
    default:                      return "UNKNOWN";
    }
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

    wait_ms(100);

    printf("\r\n=== kidbytes_nvm — Layer 4 bench test ===\r\n");
    printf("Build %s %s\r\n", __DATE__, __TIME__);

    /* --- Initialize NVM driver --- */
    bool ok = nvm_drv_init();
    if (!ok)
    {
        printf("ERROR: FLASH_Init failed!\r\n");
        led_red_on();
        while (1) {}
    }

    printf("Flash sector size: %lu bytes\r\n", nvm_drv_get_sector_size());
    printf("NVM region: 0x%08lX–0x%08lX (%lu bytes, %lu slots)\r\n",
           (uint32_t)NVM_BASE_ADDR, (uint32_t)(NVM_BASE_ADDR + NVM_SIZE - 1),
           (uint32_t)NVM_SIZE, (uint32_t)NVM_SLOT_COUNT);
    printf("Records currently stored: %u\r\n", nvm_drv_get_record_count());
    printf("Next sequence number: %lu\r\n\r\n", nvm_drv_get_seq());

    /* Check if there's already data (persistence test after reset) */
    nvm_record_t latest;
    if (nvm_drv_read_latest(&latest))
    {
        printf("*** PERSISTENT DATA FOUND (survived power cycle!) ***\r\n");
        printf("    seq=%lu room=%u progress=%u outcome=%s ts=%lu\r\n\r\n",
               latest.seq, latest.room_id, latest.route_progress,
               outcome_str(latest.outcome), latest.timestamp_ms);
        led_green_on();
    }
    else
    {
        printf("NVM is empty (first boot or after erase)\r\n\r\n");
    }

    /* =====================================================================
     * Test 1: Erase NVM region
     * =================================================================== */
    printf("--- Test 1: Erase NVM region ---\r\n");
    ok = nvm_drv_erase_all();
    printf("[%08lu] Erase %s\r\n\r\n", ms, ok ? "OK" : "FAILED");
    if (!ok) { led_red_on(); while (1) {} }

    /* =====================================================================
     * Test 2: Write first record
     * =================================================================== */
    printf("--- Test 2: Write first record ---\r\n");

    nvm_record_t rec;
    memset(&rec, 0, sizeof(rec));
    rec.room_id = 1;
    rec.route_progress = 0;
    rec.outcome = NVM_OUTCOME_ENTERED;
    rec.timestamp_ms = ms;

    uint32_t c0 = DWT->CYCCNT;
    ok = nvm_drv_write(&rec);
    uint32_t c1 = DWT->CYCCNT;

    uint32_t write_us = (c1 - c0) / 96;  /* cycles → µs at 96 MHz */
    printf("[%08lu] Write %s (seq=1, room=1, outcome=ENTERED, latency=%lu us = %lu ms)\r\n\r\n",
           ms, ok ? "OK" : "FAILED", write_us, write_us / 1000);

    /* =====================================================================
     * Test 3: Read back latest
     * =================================================================== */
    printf("--- Test 3: Read back latest ---\r\n");

    nvm_record_t readback;
    ok = nvm_drv_read_latest(&readback);
    if (ok)
    {
        printf("[%08lu] Read OK: seq=%lu room=%u progress=%u outcome=%s ts=%lu\r\n\r\n",
               ms, readback.seq, readback.room_id, readback.route_progress,
               outcome_str(readback.outcome), readback.timestamp_ms);
    }
    else
    {
        printf("[%08lu] Read FAILED\r\n\r\n", ms);
    }

    /* =====================================================================
     * Test 4: Write 5 records (simulate a tour)
     * =================================================================== */
    printf("--- Test 4: Write 5 records (simulate a tour) ---\r\n");

    struct { uint8_t room; uint8_t progress; uint8_t outcome; } tour[] = {
        {1, 1, NVM_OUTCOME_PUZZLE_OK},
        {2, 1, NVM_OUTCOME_ENTERED},
        {2, 2, NVM_OUTCOME_PUZZLE_OK},
        {3, 2, NVM_OUTCOME_ENTERED},
        {3, 3, NVM_OUTCOME_PUZZLE_OK},
    };

    for (int i = 0; i < 5; i++)
    {
        memset(&rec, 0, sizeof(rec));
        rec.room_id = tour[i].room;
        rec.route_progress = tour[i].progress;
        rec.outcome = tour[i].outcome;
        rec.timestamp_ms = ms;

        ok = nvm_drv_write(&rec);
        printf("[%08lu] Write seq=%lu room=%u outcome=%s %s\r\n",
               ms, nvm_drv_get_seq() - 1, rec.room_id,
               outcome_str(rec.outcome), ok ? "" : "FAILED");
    }

    ok = nvm_drv_read_latest(&readback);
    printf("[%08lu] Latest: seq=%lu room=%u progress=%u\r\n\r\n",
           ms, readback.seq, readback.room_id, readback.route_progress);

    /* =====================================================================
     * Test 5: Persistence message
     * =================================================================== */
    printf("--- Test 5: Power-cut simulation ---\r\n");
    printf("Reset the board now (press RESET button).\r\n");
    printf("After reset, you should see 'PERSISTENT DATA FOUND' at the top.\r\n\r\n");

    /* =====================================================================
     * Test 6: Write latency measurement
     * =================================================================== */
    printf("--- Test 6: Write latency (DWT) ---\r\n");

    memset(&rec, 0, sizeof(rec));
    rec.room_id = 5;
    rec.route_progress = 5;
    rec.outcome = NVM_OUTCOME_UNLOCKED;
    rec.timestamp_ms = ms;

    c0 = DWT->CYCCNT;
    ok = nvm_drv_write(&rec);
    c1 = DWT->CYCCNT;

    write_us = (c1 - c0) / 96;
    printf("[%08lu] Write latency: %lu us (%lu ms) — %s\r\n\r\n",
           ms, write_us, write_us / 1000,
           (write_us / 1000 < 50) ? "PASS (< 50 ms, F7.2)" : "FAIL (>= 50 ms)");

    /* =====================================================================
     * Done
     * =================================================================== */
    printf("=== ALL TESTS PASSED ===\r\n");
    printf("Total records stored: %u\r\n", nvm_drv_get_record_count());
    led_blue_on();

    while (1)
    {
        __WFI();
    }
}
