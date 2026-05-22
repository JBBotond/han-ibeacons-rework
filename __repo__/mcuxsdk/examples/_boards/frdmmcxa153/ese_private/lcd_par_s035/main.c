/*! ***************************************************************************
 *
 * \brief     Main application
 * \file      main.c
 * \author    Hugo Arends
 * \date      January 2026
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
#include <MCXA153.h>
#include <stdio.h>

#include "leds.h"
#include "serial.h"
#include "switches.h"

#include "lcd_par_s035.h"
#include "image.h"

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
static const uint32_t interval_ms = 2000;

#if (BYTES_PER_PIXEL == 3U)
// RGB888
static const color_rgb888_t color_lut_rgb888[] =
{
    { .r = 255, .g = 0,   .b = 0   }, // Red
    { .r = 0,   .g = 255, .b = 0   }, // Green
    { .r = 0,   .g = 0,   .b = 255 }, // Blue
    { .r = 255, .g = 255, .b = 0   }, // Yellow
    { .r = 0,   .g = 255, .b = 255 }, // Cyan
    { .r = 255, .g = 0,   .b = 255 }, // Magenta
    { .r = 255, .g = 255, .b = 255 }, // White
    { .r = 0,   .g = 0,   .b = 0   }  // Black
};

static const uint32_t color_lut_size =
    sizeof(color_lut_rgb888) / sizeof(color_lut_rgb888[0]);

#elif (BYTES_PER_PIXEL == 2U)
// RGB565
static const color_rgb565_t color_lut_rgb565[] =
{
    { .value = 0x001F }, // Red
    { .value = 0x07E0 }, // Green
    { .value = 0xF800 }, // Blue
    { .value = 0x07FF }, // Yellow
    { .value = 0xFFE0 }, // Cyan
    { .value = 0xF81F }, // Magenta
    { .value = 0xFFFF }, // White
    { .value = 0x0000 }  // Black
};

static const uint32_t color_lut_size =
    sizeof(color_lut_rgb565) / sizeof(color_lut_rgb565[0]);

#else
#error "Unsupported BYTES_PER_PIXEL value"
#endif

static uint8_t color_cnt = 0;

// -----------------------------------------------------------------------------
// Main application
// -----------------------------------------------------------------------------
int main(void)
{
    // 96 MHz FIRC clock selected
    SCG0->FIRCCFG = SCG_FIRCCFG_FREQ_SEL(0b101);

    // Generate an interrupt every 1 ms
    SysTick_Config(96000);

    // This project uses a custom serial implementation, since the core clock
    // in this project runs ate 96 MHz instead of the default 48 MHz.
    serial_init(115200);
    leds_init();
    sw_init();
    lcd_init();

    printf("LCD-PAR-S035 demo application");
    printf(" - %s\r\n", TARGETSTR);
    printf("Build %s %s\r\n", __DATE__, __TIME__);

    // Display image in the center of the screen
    //
    // To create an image use the convert.py script:
    // 1. Create an image: python .\convert.py .\image01.bmp image
    //                     python .\convert.py .\image08.bmp image 160 120
    // 2. Copy image.h and image.c in the project

    #if (BYTES_PER_PIXEL == 3U)

    // lcd_select_area(320/2-32, 480/2-32, 320/2+32-1, 480/2+32-1);
    lcd_select_area(320/2-80, 480/2-60, 320/2+80-1, 480/2+60-1);
    lcd_write_pixels((const uint8_t *)image_data, sizeof(image_data));

    // while(ms < 5000)
    // {}

    while(!sw3_pressed())
    {}

    #endif

    while(1)
    {
        // Wait for interrupt
        __WFI();

        // Update every interval_ms milliseconds
        if ((ms - previous_ms) >= interval_ms)
        {
            previous_ms = ms;

            for(uint32_t y=0; y<32; y++)
            {
                for(uint32_t x=0; x<32; x++)
                {
                    uint32_t idx = (y*32 + x) * BYTES_PER_PIXEL;

                    #if (BYTES_PER_PIXEL == 3U)

                    lcd_buffer[idx]   = color_lut_rgb888[color_cnt].b;
                    lcd_buffer[idx+1] = color_lut_rgb888[color_cnt].g;
                    lcd_buffer[idx+2] = color_lut_rgb888[color_cnt].r;

                    #elif (BYTES_PER_PIXEL == 2U)

                    lcd_buffer[idx]   = (uint8_t)(color_lut_rgb565[color_cnt].value >> 8);
                    lcd_buffer[idx+1] = (uint8_t)(color_lut_rgb565[color_cnt].value & 0x00FF);

                    #else

                    #error "Unsupported BYTES_PER_PIXEL value"

                    #endif

                }
            }

            // Next color
            color_cnt++;
            if (color_cnt >= color_lut_size)
            {
                color_cnt = 0;
            }

            // Update lcd in 32x32 blocks
            for(uint32_t b=0; b<15; b++)
            {
                for(uint32_t a=0; a<10; a++)
                {
                    lcd_select_area(a*32, b*32, a*32+31, b*32+31);
                    lcd_write_pixels(lcd_buffer, sizeof(lcd_buffer));
                }
            }

            printf("LCD updated to color %u\r\n", color_cnt);
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
