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
void lcd_text_helper(uint16_t x, uint16_t y, uint16_t width, uint16_t height,
    const char *str, const char *font, uint16_t color, uint16_t bg_color);

// -----------------------------------------------------------------------------
// Local variables
// -----------------------------------------------------------------------------
volatile uint32_t ms = 0;

static orientation_t orientation = ORIENTATION_0;

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
    lcd_orientation(ORIENTATION_0);
    lcd_draw_bitmap(0, lcd_height - 40, MADE_BY_ESE_BMP_WIDTH, MADE_BY_ESE_BMP_HEIGHT,
        (const uint16_t *)made_by_ese_bmp);
    lcd_set_font(Dialog_bold_16);
    uint8_t font_height = Dialog_bold_16[1];
    lcd_put_string(0, 0, "TFT LCD ILI9341 240x320", RGB_WHITE, RGB_BLACK);
    lcd_put_string(0, 1 * font_height, "Text example", RGB_WHITE, RGB_BLACK);

    lcd_set_font(Dialog_plain_12);
    font_height = Dialog_plain_12[1];
    lcd_put_string(0, 4 * font_height, "Touch the LCD to change the", RGB_WHITE, RGB_BLACK);
    lcd_put_string(0, 5 * font_height, "orientation and text appearance.", RGB_WHITE, RGB_BLACK);

    lcd_put_string(0, 7 * font_height, "Touch the LCD to start.", RGB_WHITE, RGB_BLACK);

    while(touch_detected == false)
    {}

    const char text[] =
        "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod "
        "tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim "
        "veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea "
        "commodo consequat.\n"
        "\n"
        "Duis aute irure dolor in reprehenderit in voluptate "
        "velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint "
        "occaecat cupidatat non proident, sunt in culpa qui officia deserunt "
        "mollit anim id est laborum.";

    const char *fonts[] =
    {
        Dialog_plain_12,
        Dialog_plain_12,
        Dialog_bold_16,
        Monospaced_plain_10
    };

    const uint16_t colors[][2] =
    {
        {RGB_WHITE,  RGB_BLACK},
        {RGB_BLACK,  RGB_WHITE},
        {RGB_WHITE,  RGB_TEAL},
        {RGB_RED,    RGB_GRAY}
    };

    const orientation_t orientations[] =
    {
        ORIENTATION_0,
        ORIENTATION_90,
        ORIENTATION_180,
        ORIENTATION_270
    };

    const char *info[] =
    {
        "Orientation:   0, Font: Dialog_plain_12,     Colors: White on Black",
        "Orientation:  90, Font: Dialog_plain_12,     Colors: Black on White",
        "Orientation: 180, Font: Dialog_bold_16,      Colors: White on Teal",
        "Orientation: 270, Font: Monospaced_plain_10, Colors: Red on Gray"
    };

    uint32_t cnt = 0;

    while(1)
    {
        // Wait for interrupt
        __WFI();

        // ---------------------------------------------------------------------
        // Check for touch input

        if(touch_detected)
        {
            touch_detected = false;

            lcd_get_touch();

            // Change settings
            cnt = (cnt + 1) % 4;
            orientation = orientations[cnt];
            lcd_orientation(orientation);
            lcd_clear(colors[cnt][1]);
            lcd_text_helper(20, 20, lcd_width - 40, lcd_height - 40,
                text, fonts[cnt], colors[cnt][0], colors[cnt][1]);

            printf("TFT LCD touched\r\n");
            printf("%s\r\n", info[cnt]);
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

/*!
 * \brief Helper function to display text on the LCD.
 *
 * This function is a simple helper function to display text on the LCD. It is
 * simple, because it only handles newlines and spaces, and does not do any
 * advanced text layouting. If the text is too long to fit on the LCD, it will
 * be cut off.
 *
 * \param x        The x-coordinate of the text position.
 * \param y        The y-coordinate of the text position.
 * \param width    The maximum width of the text area. If a word exceeds this
 *                 width, it will be moved to the next line.
 * \param height   The maximum height of the text area.
 * \param str      The string to display.
 * \param font     The font to use for displaying the text.
 * \param color    The color of the text.
 * \param bg_color The background color of the text.
 */
void lcd_text_helper(uint16_t x, uint16_t y, uint16_t width, uint16_t height,
    const char *str, const char *font, uint16_t color, uint16_t bg_color)
{
    lcd_set_font(font);

    uint16_t _x = x;
    uint16_t _y = y;

    while(*str != '\0')
    {
        // Determine length of the next word.
        uint32_t word_len_pixels = 0;
        uint32_t i=0;

        while(str[i] != ' ' && str[i] != '\0')
        {
            char c = str[i];
            uint8_t font_firstchar = font[2];
            uint32_t index = 4 + ((c - font_firstchar) * 4);
            uint8_t char_width = font[index + 3];

            word_len_pixels += char_width;

            i++;
        }

        // If the word is too long to be displayed at the given x coordinate, so
        // move to the next line and reset the x coordinate.
        if((_x + word_len_pixels) > (x + width))
        {
            _y += font[1];

            if(_y > (y + height))
            {
                // No more space to display text, exit the function.
                return;
            }

            _x = x;
        }

        while(*str != ' ' && *str != '\0')
        {
            if(*str == '\n')
            {
                // Newline character, move to the next line and reset the x coordinate.
                _y += font[1];

                if(_y > (y + height))
                {
                    // No more space to display text, exit the function.
                    return;
                }

                _x = x;
            }
            else
            {
                _x += lcd_put_char(_x, _y, *str, color, bg_color);
            }

            str++;
        }

        while(*str == ' ')
        {
            _x += lcd_put_char(_x, _y, *str, color, bg_color);
            str++;
        }
    }
}
