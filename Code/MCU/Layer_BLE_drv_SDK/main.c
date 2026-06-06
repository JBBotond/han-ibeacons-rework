/*! ***************************************************************************
 * \brief  kidbytes_ble — Layer 4 ble_drv bench test
 * \file   main.c
 *
 * Test: MCU autonomously scans for iBeacons every 250 ms and prints
 * the detected room + RSSI to serial (LPUART0 / USB VCOM at 115200).
 *
 * Expected output on serial monitor:
 *   [00000500] BLE: HM-10 connected
 *   [00001000] SCAN: started
 *   [00001200] SCAN: room=1 rssi=-66 matched=1/5
 *   [00001500] SCAN: started
 *   [00001700] SCAN: room=1 rssi=-63 matched=1/4 signal_low=0
 *   ...
 *
 * Wiring: HM-10 on P1_4 (RX) / P1_5 (TX) at 9600 bps.
 *****************************************************************************/
#include <MCXA153.h>
#include <stdio.h>
#include <string.h>

#include "serial.h"
#include "leds.h"
#include "config_store.h"
#include "location_engine.h"
#include "ble_drv.h"

/* -------------------------------------------------------------------------
 * Globals
 * ---------------------------------------------------------------------- */
volatile uint32_t ms = 0;

void SysTick_Handler(void) { ms++; }

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

    printf("\r\n[%08lu] kidbytes_ble — Layer 4 bench test\r\n", ms);
    printf("[%08lu] Build %s %s\r\n", ms, __DATE__, __TIME__);

    /* --- Layer 7: config + location engine --- */
    config_store_t cfg;
    config_init(&cfg);

    loc_state_t loc;
    loc_init(&loc, &cfg);

    /* --- Layer 4: BLE driver --- */
    ble_drv_state_t ble;
    bool hm10_ok = ble_drv_init(&ble, &loc);

    if (hm10_ok)
    {
        printf("[%08lu] BLE: HM-10 connected\r\n", ms);
        led_green_on();
    }
    else
    {
        printf("[%08lu] BLE: WARNING — HM-10 not responding\r\n", ms);
        led_red_on();
    }

    /* --- Main loop: scan every 250 ms (F1.1) --- */
    uint32_t next_scan = ms + BLE_SCAN_PERIOD_MS;

    while (1)
    {
        /* Time to trigger a new scan? */
        if ((int32_t)(ms - next_scan) >= 0 && !ble_drv_is_scanning(&ble))
        {
            ble_drv_start_scan(&ble);
            next_scan = ms + BLE_SCAN_PERIOD_MS;
        }

        /* Poll for incoming data and parse tokens */
        bool scan_done = ble_drv_poll(&ble);

        if (scan_done)
        {
            /* Print results */
            uint8_t room    = loc_get_room(&loc);
            int8_t  rssi    = loc_get_rssi(&loc);
            bool    low     = loc_is_signal_low(&loc);
            uint8_t matched = ble_drv_get_matched_count(&ble);

            if (matched > 0 && room != LOC_NO_ROOM)
            {
                /* Live detection — beacon is in range right now */
                printf("[%08lu] SCAN: room=%u rssi=%d matched=%u signal_low=%u\r\n",
                       ms, room, rssi, matched, low);

                /* Visual feedback: blue LED blinks on live detection */
                led_green_off();
                led_blue_on();
                uint32_t t = ms;
                while ((ms - t) < 20) {}
                led_blue_off();
                led_green_on();
            }
            else if (room != LOC_NO_ROOM && low)
            {
                /* Stale — last known room retained per F1.3, but no beacon in range */
                printf("[%08lu] SCAN: last_room=%u (stale) signal_low=1\r\n",
                       ms, room);
            }
            else
            {
                /* Never detected any room */
                printf("[%08lu] SCAN: no_room signal_low=%u\r\n", ms, low);
            }
        }

        __WFI();
    }
}
