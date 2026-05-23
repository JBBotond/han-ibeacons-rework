/*! ***************************************************************************
 *
 * \brief     Main application
 * \file      main.c
 * \author    Hugo Arends
 * \date      February 2024
 *
 * \see       NXP. (2024). MCX A153, A152, A143, A142 Reference Manual. Rev. 4,
 *            01/2024. From:
 *            https://www.nxp.com/docs/en/reference-manual/MCXAP64M96FS3RM.pdf
 *
 * \copyright 2024 HAN University of Applied Sciences. All Rights Reserved.
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
#include <stdio.h>
#include <string.h>

#include "serial.h"

#include "tft_lcd.h"
#include "animations.h"
#include "bitmaps.h"
#include "fonts.h"
#include "start.h"


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
volatile uint32_t ms = 0;
static volatile uint32_t previous_ms = 0;



//static orientation_t orientation = ORIENTATION_0;

// -----------------------------------------------------------------------------
// Main application
// -----------------------------------------------------------------------------
int main(void)
{
    // 96 MHz FIRC clock selected
    SCG0->FIRCCFG = SCG_FIRCCFG_FREQ_SEL(0b101);

    // Generate an interrupt every 1 ms
    SysTick_Config(96000);

    serial_init(115200);
    lcd_init();

    printf("TFT LCD ILI9341 240x320 example\r\n");
    printf("%s build %s %s\r\n", TARGETSTR, __DATE__, __TIME__);

    lcd_clear(RGB_BLACK);
    lcd_orientation(ORIENTATION_90);
    /*lcd_draw_bitmap(0, lcd_height - 40, MADE_BY_ESE_BMP_WIDTH, MADE_BY_ESE_BMP_HEIGHT,
                    (const uint16_t *)made_by_ese_bmp);*/
    lcd_set_font(Dialog_bold_16);
    uint8_t font_height = Dialog_bold_16[1];
    lcd_put_string(0, 0, "WELCOME", RGB_WHITE, RGB_BLACK);
    lcd_put_string(0, 1 * font_height, "Simon Says game", RGB_WHITE, RGB_BLACK);

    lcd_set_font(Dialog_plain_12);
    font_height = Dialog_plain_12[1];

    lcd_put_string(0, 9 * font_height, "Touch the LCD to start.", RGB_WHITE, RGB_BLACK);
    while (touch_detected == false)
    {
    }
    touch_detected = false;
    lcd_clear(RGB_BLACK);
    lcd_orientation(ORIENTATION_90);
    start_screen();
    while (1)
    {
        // Wait for interrupt
        __WFI();

        // ---------------------------------------------------------------------
        // Check for touch input

        if (touch_detected)
        {
            touch_detected = false;
            lcd_get_touch();
            
            
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
