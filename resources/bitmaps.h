/*! ***************************************************************************
 *
 * \brief     Bitmaps
 * \file      bitmaps.h
 * \author    Hugo Arends
 * \date      March 2026
 *
 * \remark    New bitmaps can be added by using the convert.py python script.
 *
 * \example
 *            lcd_orientation(ORIENTATION_0);
 *            lcd_clear(RGB_BLACK);
 *
 *            // Center the image
 *            lcd_draw_bitmap(
 *                (lcd_width  / 2) - (NXP_BMP_WIDTH  / 2),
 *                (lcd_height / 2) - (NXP_BMP_HEIGHT / 2),
 *                NXP_BMP_WIDTH,
 *                NXP_BMP_HEIGHT,
 *                (const uint16_t *)nxp_bmp);
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
#ifndef BITMAPS_H_
#define BITMAPS_H_

#include "tft_lcd.h"

// Bitmap dimensions and data: 120x21 pixels in BGR565 format (uint16_t)
#define MADE_BY_ESE_BMP_WIDTH  120
#define MADE_BY_ESE_BMP_HEIGHT 21
extern const uint16_t made_by_ese_bmp[MADE_BY_ESE_BMP_HEIGHT][MADE_BY_ESE_BMP_WIDTH];

// Bitmap dimensions and data: 240x82 pixels in BGR565 format (uint16_t)
#define NXP_BMP_WIDTH  240
#define NXP_BMP_HEIGHT 82
extern const uint16_t nxp_bmp[NXP_BMP_HEIGHT][NXP_BMP_WIDTH];

#define RED_BMP_WIDTH  40
#define RED_BMP_HEIGHT 113
extern const uint16_t red_bmp[RED_BMP_HEIGHT][RED_BMP_WIDTH];


#define RETARD_BMP_WIDTH  210
#define RETARD_BMP_HEIGHT 130
extern const uint16_t retard_bmp[RETARD_BMP_HEIGHT][RETARD_BMP_WIDTH];

#define SLEEPY_3_BMP_WIDTH  110
#define SLEEPY_3_BMP_HEIGHT 147
extern const uint16_t sleepy_3_bmp[SLEEPY_3_BMP_HEIGHT][SLEEPY_3_BMP_WIDTH];

#endif // BITMAPS_H_

