/*! ***************************************************************************
 *
 * \brief     TFT LCD (ILI9341 and XPT2046) driver
 * \file      tft_lcd.h
 * \author    Hugo Arends
 * \date      March 2026
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
#ifndef TFT_LCD_H
#define TFT_LCD_H

#include "lpspi_master.h"

/// \name Definitions for TFT LCD
/// \{

/*!
 * \brief Definition for the display dimensions
 */
#define LCD_WIDTH           (240)
#define LCD_HEIGHT          (320)
#define LCD_BYTES_PER_PIXEL (2)
#define LCD_BYTES           (LCD_WIDTH * LCD_HEIGHT * LCD_BYTES_PER_PIXEL)

/*!
 * \brief Helper definitions for working with RGB565 colors
 */
#define SWAP_BYTES(value) (uint16_t)(((uint16_t)(value) << 8) | ((uint16_t)(value) >> 8))

#define RGB(r,g,b)  SWAP_BYTES(((((uint16_t)(r) & 0x1F) << 11) | (((uint16_t)(g) & 0x3F) << 5) | ((uint16_t)(b) & 0x1F)))

/*!
 * \brief Color definitions for common colors in RGB565 format
 *
 * The RGB565 format uses 5 bits for red, 6 bits for green, and 5 bits for blue.
 * Each color component is represented by a value from 0 to 31 for red and blue,
 * and from 0 to 63 for green. You can use the RGB macro to create custom colors
 * by specifying the red, green, and blue components as arguments.
 * For example, RGB(31,0,0) will give you pure red, RGB(0,63,0) will give  you
 * pure green, and RGB(0,0,31) will give you pure blue. You can combine
 * different values to create a wide range of colors.
 */
#define RGB_RED     RGB(31,0,0)
#define RGB_GREEN   RGB(0,63,0)
#define RGB_BLUE    RGB(0,0,31)
#define RGB_YELLOW  RGB(31,63,0)
#define RGB_CYAN    RGB(0,63,31)
#define RGB_MAGENTA RGB(31,0,31)
#define RGB_WHITE   RGB(31,63,31)
#define RGB_BLACK   RGB(0,0,0)
#define RGB_GRAY    RGB(16,32,16)
#define RGB_ORANGE  RGB(31,32,0)
#define RGB_PURPLE  RGB(16,0,31)
#define RGB_PINK    RGB(31,16,31)
#define RGB_BROWN   RGB(16,8,0)
#define RGB_LIME    RGB(16,63,0)
#define RGB_NAVY    RGB(0,0,16)
#define RGB_TEAL    RGB(0,32,16)

/**
 * \brief Orientation of the display
 *
 * Orientation can be set to 0, 90, 180 or 270 degrees. This orientation
 * setting is used for both display and touch calculation.
 */
typedef enum orientation
{
    ORIENTATION_0 = 0,
    ORIENTATION_90 = 1,
    ORIENTATION_180 = 2,
    ORIENTATION_270 = 3,
} orientation_t;

/// \}

/**
 * \brief LCD frame buffer
 *
 * Buffer for storing pixel data. A 240x320 framebuffer does not fit in the
 * microntroller SRAM. The GCD of the 240x320 is 80, which is a good trade-off
 * between SRAM usage ((80x80) / 1024 = 6.25 kB) vs buffer size. If less SRAM
 * should be used, try other divisors of 240 and 320, such as 40, 20, 16, and 8.
 *
 * Important note. When writing fonts, the number of pixels in a character
 * cannot exceed the size of this frame buffer, otherwise the character cannot
 * be displayed. This is a runtime check in the lcd_put_char function, but it
 * is recommended to choose a font that fits well within the size of the frame
 * buffer to avoid issues.
 */
#define LCD_FRAMEBUFFER_WIDTH  (80)
#define LCD_FRAMEBUFFER_HEIGHT (80)
extern uint16_t lcd_framebuffer[LCD_FRAMEBUFFER_WIDTH * LCD_FRAMEBUFFER_HEIGHT];

/*!
 * \brief Width and height, will be updated on orientation change
 */
extern int16_t lcd_width;
extern int16_t lcd_height;

/*!
 * \brief (x,y) coordinates of the last detected touch
 */
extern int16_t lcd_touch_x;
extern int16_t lcd_touch_y;
extern int16_t lcd_touch_z;

// Function prototypes
void lcd_init(void);
void lcd_backlight(bool on);
void lcd_inverse(bool on);
void lcd_orientation(orientation_t orientation);
void lcd_set_area(uint16_t start_x, uint16_t start_y, uint16_t width, uint16_t height);
void lcd_write_pixels(uint16_t *pixels, uint32_t length);
void lcd_clear(uint16_t color);
void lcd_get_touch(void);

void lcd_set_font(const char *f);
uint16_t lcd_put_char(uint16_t x, uint16_t y, char character, uint16_t text_color, uint16_t bg_color);
void lcd_put_string(uint16_t x, uint16_t y, const char *str, uint16_t text_color, uint16_t bg_color);

void lcd_draw_bitmap(uint16_t x, uint16_t y, uint16_t width, uint16_t height, const uint16_t *bitmap);
void lcd_draw_animation(uint16_t x, uint16_t y, uint8_t width, uint8_t height, const unsigned char *frame,  uint16_t color, uint16_t bg_color);

#endif // TFT_LCD_H
