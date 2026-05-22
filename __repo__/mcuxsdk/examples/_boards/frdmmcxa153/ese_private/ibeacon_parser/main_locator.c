/*! ***************************************************************************
 *
 * \brief     Main application
 * \file      main.c
 * \author    Hugo Arends
 * \date      February 2025
 *
 * \see       NXP. (2024). MCX A153, A152, A143, A142 Reference Manual. Rev. 4,
 *            01/2024. From:
 *            https://www.nxp.com/docs/en/reference-manual/MCXAP64M96FS3RM.pdf
 *
 * \copyright 2025 HAN University of Applied Sciences. All Rights Reserved.
 *            \n\n
 *            Permission is hereby granted, free of charge, to any person
 *            obtaining a copy of this software and associated documentation
 *            files (the "Software"), to deal in the Software without
 *            restriction, including without limitation the rights to use,
 *            copy, modify, merge, publish, distribute, sublicense, and/or sell
 *            copies of the Software, and to permit persons to whom the
 *            Software is furnished to do so, subject to the following
 *            conditions:
 *            \n\n
 *            The above copyright notice and this permission notice shall be
 *            included in all copies or substantial portions of the Software.
 *            \n\n
 *            THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 *            EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 *            OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 *            NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 *            HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 *            WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 *            FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 *            OTHER DEALINGS IN THE SOFTWARE.
 *
 *****************************************************************************/
#include <MCXA153.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "leds.h"
#include "serial.h"

#include "lpi2c0_controller_dma.h"
#include "lpuart2.h"
#include "ssd1306.h"

// -----------------------------------------------------------------------------
// Local type definitions
// -----------------------------------------------------------------------------
#ifdef DEBUG
#define TARGETSTR "Debug"
#else
#define TARGETSTR "Release"
#endif

typedef struct location
{
    uint16_t major;
    uint16_t minor;
    char    *major_str;
    char    *minor_str;
}location_t;

// -----------------------------------------------------------------------------
// Local function prototypes
// -----------------------------------------------------------------------------
void lpuart2_send_string(const char *str);

// -----------------------------------------------------------------------------
// Local variables
// -----------------------------------------------------------------------------
static volatile uint32_t ms = 0;
static uint32_t next_scan_ms = 1000;

static char scan_str[128] = {0};
static uint32_t scan_str_cnt = 0;

static uint32_t current_location = 0;
static uint32_t target_location = 1;

static location_t locations[] =
{
    {0x0000, 0x0000, "unknown", "unknown"},
    {0x0AEA, 0x0212, "AEA", "H212"},
    {0x0AEA, 0x0214, "AEA", "H214"},
    {0x0AEA, 0x0228, "AEA", "H228"},
};

static uint32_t locations_cnt = sizeof(locations) / sizeof(locations[0]);

static int8_t best_rssi_in_scan = -100;

// -----------------------------------------------------------------------------
// Main application
// -----------------------------------------------------------------------------
int main(void)
{
    leds_init();
    serial_init(115200);
    lpuart2_init(9600);
    ssd1306_init();

    // Set initial message
    ssd1306_setorientation(1);
    ssd1306_drawbitmap(made_by_ese);
    ssd1306_update();

    // Generate an interrupt every 1 ms
    SysTick_Config(48000);

    printf("\r\n");
    printf("\r\n");
    printf("[%08lu] iBeacon locator - %s\r\n", ms, TARGETSTR);
    printf("[%08lu] Build %s %s\r\n", ms, __DATE__, __TIME__);

    while(ms < 1000)
    {}

    printf("[%08lu] >> Checking if HM-10 module connected\r\n", ms);

    // Check if HM-10 connected
    lpuart2_send_string("AT\r\n");

    while(lpuart2_rxcnt() < 4)
    {}

    char data[4] = {0};

    for(int i = 0; i < 4; i++)
    {
        data[i] = (char)lpuart2_getchar();
    }

    if((data[0] == 'O') && (data[1] == 'K') && (data[2] == '\r') && (data[3] == '\n'))
    {
        printf("[%08lu] >> HM-10 module connected\r\n", ms);
    }
    else
    {
        printf("[%08lu] >> ERROR HM-10 module NOT connected\r\n", ms);

        // Red LED on
        GPIO3->PCOR = (1<<12);

        while(1)
        {}
    }

    ssd1306_clearscreen();
    ssd1306_setfont(Monospaced_plain_10);
    ssd1306_putstring(0, 0, "iBeacon locator");
    ssd1306_drawline(0, 12, 120, 12);
    ssd1306_putstring(0, 13, "Location: ");
    ssd1306_putstring(60, 13, locations[current_location].major_str);
    ssd1306_putstring(60, 26, locations[current_location].minor_str);
    ssd1306_putstring(0, 39, "Target  :");
    ssd1306_putstring(60, 39, locations[target_location].major_str);
    ssd1306_putstring(60, 52, locations[target_location].minor_str);
    ssd1306_update();

    while(1)
    {
        if(ms >= next_scan_ms)
        {
            next_scan_ms = 0xFFFFFFFF;

            printf("\r\n");
            printf("[%08lu] >> Sending scan command\r\n", ms);
            lpuart2_send_string("AT+DISI?\r\n");

            memset(scan_str, 0, sizeof(scan_str));
            scan_str_cnt = 0;

            current_location = 0;
            best_rssi_in_scan = -100;
        }

        // Data available from iBeacon module?
        if(lpuart2_rxcnt() > 0)
        {
            // Get the data from LPUART2
            uint8_t data = lpuart2_getchar();

            // lpuart0_putchar(data);

            if(data == '\n')
            {
                // Ignore linefeeds
            }
            else if (data == '\r')
            {
                // End of response line
                scan_str[scan_str_cnt++] = '\0';

                // For debugging
                // printf("[%08lu] %s\r\n", ms, scan_str);

                // Reset for next string
                scan_str_cnt=0;

                static uint32_t disc_cnt = 0;
                static uint32_t ibeacon_cnt = 0;

                const char *disis = "OK+DISIS";
                const char *disc = "OK+DISC:";
                const char *disce = "OK+DISCE";

                // P0: Factory ID
                // P1: iBeacon UUID
                // P2: Major/Minor/TxPower
                // P3: MAC
                // P4: RSSI
                const char *P0 = "4C000215";
                const char *P1 = "74278BDAB64445208F0C720EAF059935";

                if(strncmp(scan_str, disc, strlen(disc)) == 0)
                {
                    disc_cnt++;

                    if(strncmp(&scan_str[8], P0, strlen(P0)) == 0)
                    {
                        // Valid factory ID

                        if(strncmp(&scan_str[17], P1, strlen(P1)) == 0)
                        {
                            // Valid iBeacon UUID

                            ibeacon_cnt++;

                            scan_str[60] = '\0';
                            int8_t txpower = (int8_t)strtol(&scan_str[58], NULL, 16);

                            scan_str[58] = '\0';
                            uint16_t minor = (uint16_t)strtol(&scan_str[54], NULL, 16);

                            scan_str[54] = '\0';
                            uint16_t major = (uint16_t)strtol(&scan_str[50], NULL, 16);

                            int32_t rssi = atoi(&scan_str[74]);

                            // Valid iBeacon found
                            printf("[%08lu] >> iBeacon found\r\n", ms);
                            printf("[%08lu]    - Major   : 0x%04X\r\n", ms, major);
                            printf("[%08lu]    - Minor   : 0x%04X\r\n", ms, minor);
                            printf("[%08lu]    - TxPower : %d dBm\r\n", ms, txpower);
                            printf("[%08lu]    - RSSI    : %ld dBm\r\n", ms, rssi);

                            for(uint32_t i = 1; i < locations_cnt; i++)
                            {
                                if((major == locations[i].major) &&
                                   (minor == locations[i].minor) &&
                                   (rssi > -70))
                                {
                                    if(rssi > best_rssi_in_scan)
                                    {
                                        best_rssi_in_scan = rssi;

                                        current_location = i;

                                        printf("[%08lu]    - Location: %s %s\r\n", ms,
                                            locations[current_location].major_str,
                                            locations[current_location].minor_str);
                                    }
                                }
                            }
                        }
                    }
                }
                else if(strncmp(scan_str, disis, strlen(disis)) == 0)
                {
                    disc_cnt = 0;
                    ibeacon_cnt = 0;

                    printf("[%08lu] >> Receiving scan results\r\n", ms);
                }
                else if(strncmp(scan_str, disce, strlen(disce)) == 0)
                {
                    // LED on to indicate scan end
                    led_blue_on();

                    printf("[%08lu] >> Scan ended, %lu iBeacons in %lu BLE devices\r\n",
                        ms, ibeacon_cnt, disc_cnt);

                    if(current_location == target_location)
                    {
                        printf("[%08lu] >> Current location is target\r\n", ms);

                        target_location =
                            (target_location == (locations_cnt - 1)) ?
                            (locations_cnt - 1) : target_location + 1;

                        printf("[%08lu] >> New target is %s %s\r\n", ms,
                            locations[target_location].major_str,
                            locations[target_location].minor_str);
                    }

                    ssd1306_clearscreen();
                    ssd1306_setfont(Monospaced_plain_10);
                    ssd1306_putstring(0, 0, "iBeacon locator");
                    ssd1306_drawline(0, 12, 120, 12);
                    ssd1306_putstring(0, 13, "Location: ");
                    ssd1306_putstring(60, 13, locations[current_location].major_str);
                    ssd1306_putstring(60, 26, locations[current_location].minor_str);
                    ssd1306_putstring(0, 39, "Target  :");
                    ssd1306_putstring(60, 39, locations[target_location].major_str);
                    ssd1306_putstring(60, 52, locations[target_location].minor_str);
                    ssd1306_update();

                    next_scan_ms = ms + 500;

                    printf("[%08lu] >> Scheduled next scan: %lu ms\r\n", ms, next_scan_ms);

                    // LED off to indicate scan end
                    led_blue_off();
                }
                else
                {
                    printf("[%08lu] >> Unexpected input\r\n", ms);

                    ssd1306_clearscreen();
                    ssd1306_setfont(Monospaced_plain_10);
                    ssd1306_putstring(0, 0, "iBeacon locator");
                    ssd1306_drawline(0, 12, 120, 12);
                    ssd1306_putstring(0, 13, "Location: ");
                    ssd1306_putstring(60, 13, locations[0].major_str);
                    ssd1306_putstring(60, 26, locations[0].minor_str);
                    ssd1306_putstring(0, 39, "Target  :");
                    ssd1306_putstring(60, 39, locations[target_location].major_str);
                    ssd1306_putstring(60, 52, locations[target_location].minor_str);
                    ssd1306_update();

                    next_scan_ms = ms + 5000;

                    printf("[%08lu] >> Scheduled next scan: %lu ms\r\n", ms, next_scan_ms);
                }
            }
            else
            {
                // Add to scan data string
                scan_str[scan_str_cnt++] = data;

                // // Toggle blue LED to show activity
                // if(scan_str_cnt % 10 == 0)
                // {
                //     GPIO3->PCOR = (1<<0);
                // }
                // else
                // {
                //     GPIO3->PSOR = (1<<0);
                // }

                static uint32_t progress = 0;

                if(scan_str_cnt == 1)
                {
                    progress = (progress + 1) % 4;

                    if(progress == 0)
                    {
                        ssd1306_putstring(120, 0, "|");
                    }
                    else if(progress == 1)
                    {
                        ssd1306_putstring(120, 0, "/");
                    }
                    else if(progress == 2)
                    {
                        ssd1306_putstring(120, 0, "-");
                    }
                    else if(progress == 3)
                    {
                        ssd1306_putstring(120, 0, "\\");
                    }

                    ssd1306_update();
                }
            }
        }
    }
}

// -----------------------------------------------------------------------------
// Local function implementation
// -----------------------------------------------------------------------------
void SysTick_Handler(void)
{
    ms++;
}

void lpuart2_send_string(const char *str)
{
    for(uint32_t i=0; str[i] != '\0'; i++)
    {
        lpuart2_putchar(str[i]);
    }
}
