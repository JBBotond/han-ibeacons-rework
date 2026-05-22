/*! ***************************************************************************
 *
 * \brief     OLED (SSD1306) driver
 * \file      oled.h
 * \author    Hugo Arends
 * \date      January 2026
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
#ifndef OLED_H
#define OLED_H

#include "lpi2c.h"
#include "animations.h"
#include "fonts.h"
#include "bitmaps.h"

/// \name Definitions for OLED
/// \{

/*!
 * \brief Definition for the logic value of the SA0 pin of the OLED display
 */
#define OLED_SA0     (0)

/*!
 * \brief Definition for the slave address
 */
#define OLED_ADDRESS (0x3C | OLED_SA0)

/*!
 * \brief Definition for the display dimensions
 */
#define OLED_WIDTH   (128)
#define OLED_HEIGHT  (64)
#define OLED_BYTES   (OLED_WIDTH * OLED_HEIGHT / 8)

/*!
 * \brief Definition for command or data
 */
#define OLED_COMMAND (0x00)
#define OLED_DATA    (0x40)

/// \}

/// Value for a pixel
typedef enum oled_pixel
{
    OLED_PIXEL_ON,  ///< Pixel is on
    OLED_PIXEL_OFF, ///< Pixel is off
}
oled_pixel_t;

// Function prototypes
void oled_init(void);
void oled_update(void);

void oled_clearscreen(void);
void oled_setfont(const char *f);
void oled_setorientation(const uint8_t orientation);
void oled_setinverse(const uint8_t inv);
void oled_setcontrast(const uint8_t contrast);

void oled_goto(const uint16_t x, const uint16_t y);
void oled_setpixel(const uint16_t x, const uint16_t y, const oled_pixel_t val);

void oled_putchar(const char c);
void oled_putstring(const uint16_t x, const uint16_t y, const char *str);
void oled_drawline(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);
void oled_drawbitmap(const unsigned char *bitmap);
void oled_drawanimation(uint16_t x, uint16_t y, const unsigned char *frame, uint8_t w, uint8_t h);

void oled_terminal(const char *str);

#endif // OLED_H
