/*! ***************************************************************************
 *
 * \brief     Main application
 * \file      main.c
 * \author    Hugo Arends
 * \date      January 2026
 *
 *            FRDM-MCXA153              Micro SDCARD Adapter Module
 *            ------------+            +---------------------------+
 *                        |            |                           |
 *                    3V3 +------------+ 3V3                       |
 *                    GND +------------+ GND                       |
 *                        |            |                           |
 *              P1_0/SDO  +------------+ MOSI                      |
 *              P1_2/SDI  +------------+ MISO                      |
 *              P1_1/SCK  +------------+ SCK                       |
 *              P1_3/GPIO +------------+ CS                        |
 *                        |            |                           |
 *            ------------+            +---------------------------+
 *
 * \see       NXP. (2024). MCX A153, A152, A143, A142 Reference Manual. Rev. 4,
 *            01/2024. From:
 *            https://www.nxp.com/docs/en/reference-manual/MCXAP64M96FS3RM.pdf
 *
 * \copyright 2026 HAN University of Applied Sciences. All Rights Reserved.
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
#include <board.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>

#include "lcd.h"
#include "leds.h"
#include "serial.h"
#include "switches.h"

// -----------------------------------------------------------------------------
// Local type definitions
// -----------------------------------------------------------------------------
#ifdef DEBUG
#define TARGETSTR "Debug"
#else
#define TARGETSTR "Release"
#endif

// -----------------------------------------------------------------------------
// Local function prototypes
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Local variables
// -----------------------------------------------------------------------------
// Do not declare static, because it is reused in diskio.c file for creating
// delays
volatile uint32_t ms = 0;
static volatile uint32_t previous_ms = 0;
static const uint32_t interval_ms = 1000;

// -----------------------------------------------------------------------------
// Main application
// -----------------------------------------------------------------------------
int main(void)
{
    // Generate an interrupt every 1 ms
    SysTick_Config(48000);

    leds_init();
    serial_init(115200);
    sw_init();
    lcd_init();

    printf("1602 LCD I2C example\r\n");
    printf("%s build %s %s\r\n", TARGETSTR, __DATE__, __TIME__);

    lcd_backlight(on);
    lcd_set_cursor(0,1);
    lcd_put("made by ESE");

    while(1)
    {
        uint32_t current_ms = ms;

        // --------------------------------------------------------------------
        if((current_ms - previous_ms) >= interval_ms)
        {
            previous_ms = current_ms;

            printf("[%08lu] Timeout\r\n", ms);

            char str[17];
            sprintf(str, "%08lu", ms/1000);
            lcd_set_cursor(0, 0);
            lcd_put(str);
        }

        // Switch pressed?
        if(sw3_pressed())
        {
            printf("[%08lu] SW3 pressed\r\n", ms);

            lcd_backlight(off);
        }

        // Switch pressed?
        if(sw2_pressed())
        {
            printf("[%08lu] SW2 pressed\r\n", ms);

            lcd_backlight(on);
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
