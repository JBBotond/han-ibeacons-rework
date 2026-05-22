/*! ***************************************************************************
 *
 * \brief     Low level driver for the Low Power Serial Peripheral Interface
 *            (LPSPI) in master mode controlling an LCD-PAR-S035 display with
 *            ST7796S controller.
 * \file      lcd_par_s035.c
 * \author    Hugo Arends
 * \date      December 2025
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
 ******************************************************************************/
#ifndef LCD_PAR_S035_H
#define LCD_PAR_S035_H

#include <MCXA153.h>
#include <stdbool.h>

// -----------------------------------------------------------------------------
// Shared type definitions
// -----------------------------------------------------------------------------

// Set bytes per pixel (2 for RGB565, 3 for RGB888)
#define BYTES_PER_PIXEL 3U

/**
 * \brief RGB888 color structure
 *
 * Structure representing a 24-bit RGB color with 8 bits per channel.
 */
typedef struct color_rgb888
{
    uint8_t b; ///< Blue channel (0-255)
    uint8_t g; ///< Green channel (0-255)
    uint8_t r; ///< Red channel (0-255)
} color_rgb888_t;

/**
 * \brief RGB565 color structure
 *
 * Structure representing a 16-bit RGB color with 5 bits for red, 6 bits for
 * green, and 5 bits for blue.
 */
typedef struct color_rgb565
{
    uint16_t value; ///< 16-bit packed BGR565 color value
} color_rgb565_t;

/**
 * \brief LCD pixel format enumeration
 *
 * Defines the available pixel formats for the LCD display.
 * Note: Does not affect interface width.
 */
typedef enum lcd_pixel_format
{
    LCD_PixelFormatRGB444 = 3U, ///< RGB444 format, 12 bits per pixel
    LCD_PixelFormatRGB565 = 5U, ///< RGB565 format, 16 bits per pixel
    LCD_PixelFormatRGB666 = 6U, ///< RGB666 format, 18 bits per pixel
    LCD_PixelFormatRGB888 = 7U, ///< RGB888 format, 24 bits per pixel (internally dithered to 18 bits)
} lcd_pixel_format_t;

/**
 * \brief LCD display orientation enumeration
 *
 * Defines the available display rotation modes.
 */
typedef enum lcd_orientation_mode
{
    LCD_Orientation0   = 0x00U, ///< No rotation (0 degrees)
    LCD_Orientation90  = 0x60U, ///< Rotated 90 degrees clockwise
    LCD_Orientation180 = 0xC0U, ///< Rotated 180 degrees
    LCD_Orientation270 = 0xA0U, ///< Rotated 270 degrees clockwise (90 degrees counter-clockwise)
} lcd_orientation_mode_t;

// -----------------------------------------------------------------------------
// Shared variables
// -----------------------------------------------------------------------------

/**
 * \brief LCD frame buffer
 *
 * Buffer for storing pixel data (32x32 pixels in RGB888 format).
 */
extern uint8_t lcd_buffer[32 * 32 * BYTES_PER_PIXEL];

// -----------------------------------------------------------------------------
// Shared function prototypes
// -----------------------------------------------------------------------------

/**
 * \brief Initialize the LCD display
 *
 * Initializes the LCD hardware and communication interface.
 */
void lcd_init(void);

/**
 * \brief Perform software reset of the LCD
 *
 * Resets the LCD controller via software command.
 */
void lcd_software_reset(void);

/**
 * \brief Select a rectangular area on the display
 *
 * \param startX Starting X coordinate
 * \param startY Starting Y coordinate
 * \param endX   Ending X coordinate
 * \param endY   Ending Y coordinate
 */
void lcd_select_area(uint16_t startX, uint16_t startY, uint16_t endX, uint16_t endY);

/**
 * \brief Write pixel data to the display
 *
 * \param pixels Pointer to array of RGB888 pixel data
 * \param length Number of pixels to write
 */
void lcd_write_pixels(const uint8_t* pixels, uint32_t length);

/**
 * \brief Set the pixel format of the display
 *
 * \param pixelFormat Desired pixel format
 */
void lcd_set_pixel_format(lcd_pixel_format_t pixelFormat);

/**
 * \brief Set the display orientation
 *
 * \param orientationMode Desired orientation (rotation)
 */
void lcd_set_orientation(lcd_orientation_mode_t orientationMode);

/**
 * \brief Flip the display horizontally and/or vertically
 *
 * \param horizontal True to flip horizontally
 * \param vertical   True to flip vertically
 */
void lcd_flip(bool horizontal, bool vertical);

/**
 * \brief Enable or disable display color inversion
 *
 * \param invert True to invert colors, false for normal colors
 */
void lcd_set_invert_display(bool invert);

/**
 * \brief Enable or disable sleep mode
 *
 * \param enable True to enter sleep mode, false to exit sleep mode
 */
void lcd_sleep_mode(bool enable);

/**
 * \brief Enable or disable the display
 *
 * \param enable True to turn display on, false to turn display off
 */
void lcd_display_enable(bool enable);

/**
 * \brief Clear the entire display
 *
 * Clears all pixels on the display to black.
 */
void lcd_clear(void);

#endif // LCD_PAR_S035_H
