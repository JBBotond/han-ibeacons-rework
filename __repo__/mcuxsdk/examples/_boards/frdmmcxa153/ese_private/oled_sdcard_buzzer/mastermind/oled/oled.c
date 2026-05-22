/*! ***************************************************************************
 *
 * \brief     OLED (SSD1306) driver
 * \file      oled.c
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
#include "oled.h"

#include <string.h>

// This driver assumes that ms exists and is incremented every 1 ms
extern volatile uint32_t ms;

/*!
 * \brief Frame buffer
 *
 * The frame buffer holds all the data. The frame buffer is only written to the
 * Oled display when the function oled_update() is called.
 */
static uint8_t _framebuffer[OLED_BYTES];

/*!
 * \brief Pointer to the selected font
 *
 * The available fonts are in fonts.h and fonts.c.
 *
 * Note that Monospaced_plain_10 is the default font and should not be removed
 * from fonts.c and fonts.h.
 */
static const char *_font = Monospaced_plain_10;

/*!
 * \brief Local x value
 */
static uint16_t _x = 0;

/*!
 * \brief Local y value
 */
static uint16_t _y = 0;

/*!
 * \brief List of commands that will be send to the Oled display upon
 *        initialisation
 *
 * This list was created based on the flowchart given in the SSD1306 datasheet.
 * Refer to the SSD1306 datasheet for a description of all commands
 */
static uint8_t _oled_init_commands[] =
{
    0xAE,             // Display OFF
    0x20, 0x00,       // Horizontal addressing mode
    0x21, 0x00, 0x7F, // Column Address start and end (DEFAULT)
    0x22, 0x00, 0x07, // Page address start and end (DEFAULT)
    0xA8, 0x3F,       // Multiplex Ratio 63 (DEFAULT)
    0xD3, 0x00,       // Display offset 0 (DEFAULT)
    0x40,             // Display start line 0 (DEFAULT)
    0xA0,             // Segment Re-map (DEFAULT)
    0xC0,             // COM output scan direction normal (DEFAULT)
    0xDA, 0x12,       // COM pins alternative configuration
    0x81, 0xFF,       // Contrast control 256
    0xA4,             // Entire display on
    0xA6,             // Set normal display
    0xD5, 0xF0,       // Display Clock divide ratio/oscillator frequency
    0xDB, 0x20,       // V COMH deselect level ~0.77 x Vcc (DEFAULT)
    0x8D, 0x14,       // Enable charge pump
    0xAF,             // Display ON
};

/*!
 * \brief Sends a sequence of initialisation commands to the Oled display
 *
 * Refer to the SSD1306 datasheet for a description of all possible commands
 */
void oled_init(void)
{
    // Clear the frame buffer
    for(uint32_t i=0; i<OLED_BYTES; ++i)
    {
        _framebuffer[i] = 0;
    }

    // Make sure to wait for 200 ms after power on
    while(ms < 200)
    {}

    // Initialize the I2C module
    lpi2c_controller_init();

    // Initialize the OLED display
    lpi2c_write(OLED_ADDRESS, OLED_COMMAND, _oled_init_commands,
        sizeof(_oled_init_commands));
}

/*!
 * \brief Sends the framebuffer to the Oled display
 *
 * The column and page addresses are set to the initial position.
 * Then the framebuffer is transferred.
 *
 * The total number of bytes to transfer is equal to:
 * - address byte + 6 command bytes
 * - address byte + OLED_BYTES data bytes
 *
 * The transmission of a single byte takes 1/375000 * 9 = 24 us
 *
 * Example for 128 x 64 display:
 * \n
 * 24 us * 1032 bytes = 24.768 ms is the total theoretical minimum time it takes
 * to send the frame buffer to the Oled display. Measurements show that it
 * actually takes approximately 28 ms.
 *
 * Notice that this is a non-blocking transfer.
 */
void oled_update(void)
{
    uint8_t data[] =
    {
        0x21, 0x00, 0x7F, // Column Address start and end (DEFAULT)
        0x22, 0x00, 0x07, // Page address start and end (DEFAULT)
    };

    lpi2c_write(OLED_ADDRESS, OLED_COMMAND, data, sizeof(data));

    // The display requires an idle time of 1.3 us (see table 13-6 in the
    // datasheet) between I2C transfers.
    uint32_t current_ms = ms;
    while(ms < current_ms + 2)
    {}

    // Write the frame buffer to the device
    lpi2c_write(OLED_ADDRESS, OLED_DATA, _framebuffer, sizeof(_framebuffer));
}

/*!
 * \brief Sets the font
 *
 * Changing a font doesn't change what is already on the Oled display.
 * This font will be used when writing new characters. Fonts should be located
 * in the files fonts.c and fonts.h.
 *
 * \param[in]  f  A pointer to a font
 */
void oled_setfont(const char *f)
{
    _font = f;
}

/*!
 * \brief Sets the display's orientation
 *
 * This function either sets default display orientation or flips the display
 * both horizontally and vertically.
 *
 * If \p orientation = 0: default display orientation
 *
 * If \p orientation > 0: flipped both horizontally and vertically
 *
 * \param[in]  orientation  Display orientation
 */
void oled_setorientation(const uint8_t orientation)
{
    // The display requires an idle time before updating the orientation
    uint32_t current_ms = ms;
    while(ms < current_ms + 2)
    {}

    uint8_t data[2];

    if(orientation)
    {
        data[0] = 0xA1;
        data[1] = 0xC8;
    }
    else
    {
        data[0] = 0xA0;
        data[1] = 0xC0;
    }

    lpi2c_write(OLED_ADDRESS, OLED_COMMAND, data, sizeof(data));
}

/*!
 * \brief Sets the display's inverse mode
 *
 * This function enables or disables the display's inverse mode
 *
 * If \p inv = 0: inverse mode disabled
 *
 * If \p inv > 0: inverse mode enabled
 *
 * \param[in]  inv  inverse mode
 */
void oled_setinverse(const uint8_t inv)
{
    uint8_t data = (inv) ? 0xA7 : 0xA6;

    lpi2c_write(OLED_ADDRESS, OLED_COMMAND, &data, sizeof(data));
}

/*!
 * \brief Clears the display
 *
 * Clears all data in the frame buffer.
 * Call the function oled_update() to actually show the result.
 */
void oled_clearscreen(void)
{
    memset(_framebuffer, 0x00, sizeof(_framebuffer));
}

/*!
 * \brief Sets the display's contrast
 *
 * This function sets the contrast between 0 and 255
 *
 * \param[in]  contrast  Contrast value
 */
void oled_setcontrast(const uint8_t contrast)
{
    uint8_t data[2] =
    {
        0x81, 0x00,
    };

    data[1] = contrast;

    lpi2c_write(OLED_ADDRESS, OLED_COMMAND, data, sizeof(data));
}

/*!
 * \brief Sets x and y
 *
 * This function updates the current (x,y) value
 *
 * \param[in]  x  New value for x
 * \param[in]  y  New value for y
 */
void oled_goto(const uint16_t x, const uint16_t y)
{
    if((x >= OLED_WIDTH) || (y >= OLED_HEIGHT))
    {
        return;
    }

    _x = x;
    _y = y;
}

/*!
 * \brief Sets the pixel at (x,y) to val
 *
 * This function sets the value of the pixel at location (x,y).
 * Call the function oled_update() to actually show the result.
 *
 * \param[in]  nx   x-value
 * \param[in]  ny   y-value
 * \param[in]  val  Pixel value
 */
void oled_setpixel(const uint16_t x, const uint16_t y, const oled_pixel_t val)
{
    if(val == OLED_PIXEL_ON)
    {
        _framebuffer[x + (y / 8) * OLED_WIDTH] |= 1 << (y % 8);
    }
    else
    {
        _framebuffer[x + (y / 8) * OLED_WIDTH] &= ~(1 << (y % 8));
    }
}

/*!
 * \brief Displays a character at the current (x,y) position
 *
 * Writes a character into the frame buffer, using the selected font.
 * The (x,y) location is the top-left location of the character.
 * After writing a char to the frame buffer, y is not updated, only x.
 * Call the function oled_update() to actually show the result.
 *
 * \param[in]  c  Character to display
 */
void oled_putchar(const char c)
{
    // Get the first four parameters from the font
    // uint8_t font_width = font[0];
    uint8_t font_height = _font[1];
    uint8_t font_firstchar = _font[2];
    uint8_t font_numchars = _font[3];

    // Calculate the index in the jump table
    uint32_t index = 4 + ((c - font_firstchar) * 4);

    // Read the data from the jump table
    uint8_t offset1 = _font[index];
    uint8_t offset2 = _font[index + 1];
    uint8_t n_bytes = _font[index + 2];
    uint8_t char_width = _font[index + 3];

    // Calculate the index in the data table
    index = 4 + (font_numchars * 4) + (256UL * offset1) + offset2;

    // Calculate the number of bytes for each column
    uint8_t bytes_per_col = (font_height / 8);
    if((font_height % 8) > 0)
    {
        bytes_per_col++;
    }

    // Calculate the total number of bytes for this character
    uint8_t bytes_total = bytes_per_col * char_width;

    // Temporary variable for y position
    uint16_t y_tmp = _y;

    // Temporary variable counting the character height in pixels
    uint8_t height = 0;

    // Loop all bytes in a character
    for(uint32_t n=0; n<bytes_total; n++)
    {
        // Initially a byte is cleared
        char data = 0;

        // Is there data available in the character table for this byte?
        if(n < n_bytes)
        {
            data = _font[index + n];
        }

        // Next column?
        if((n % bytes_per_col) == 0)
        {
            y_tmp = _y;
            _x++;

            // Stop if the x value is outside screen boundaries
            if(_x >= OLED_WIDTH)
            {
                return;
            }

            height = 0;
        }

        // Loop all bits in the byte and update the frame buffer
        for(uint8_t mask = 1; mask != 0; mask <<= 1)
        {
            oled_pixel_t val = (data & mask) ? OLED_PIXEL_ON : OLED_PIXEL_OFF;
            oled_setpixel(_x, y_tmp, val);
            y_tmp++;

            // Stop if the _y value is outside screen boundaries
            if(y_tmp >= OLED_HEIGHT)
            {
                break;
            }

            // Stop if the font height has been reached
            height++;
            if(height >= font_height)
            {
                break;
            }
        }
    }
}

/*!
 * \brief Displays a string of characters starting at position (x,y).
 *
 * Writes a string of character into the frame buffer, using the selected font.
 * After writing a char to the frame buffer, y is not updated, only x.
 * Call the function oled_update() to actually show the result.
 *
 * A '\n' character moves the (x,y) position to the next line, taking the
 * current selected font height into account.
 *
 * A '\r' character is ignored.
 *
 * \param[in]  x    x-value of the string
 * \param[in]  y    y-value of the string
 * \param[in]  str  '\0' terminated string
 */
void oled_putstring(const uint16_t x, const uint16_t y, const char *str)
{
    uint8_t delta = 0;

    oled_goto(x, y);

    uint32_t i=0;
    while(str[i] != '\0')
    {
        if(str[i] == '\n')
        {
            // Go to a new line
            // Set the original x value and increment the y value by the font
            // height
            delta += _font[1];
            oled_goto(x, y+delta);
        }
        else if(str[i] == '\r')
        {
            // Ignore
        }
        else
        {
            oled_putchar(str[i]);
        }

        i++;
    }
}

/*!
 * \brief Emulates a mini terminal
 *
 * Writes a string of characters into the lowest line of the frame buffer, using
 * the selected font. The function updates the Oled display by calling the
 * function oled_update().
 *
 * A '\n' character scrolls all lines one line up, taking the current selected
 * font height into account and clears the bottom line.
 *
 * A '\r' character moves the x position to the beginning of the line. This
 * means that previous written characters will be overwritten.
 *
 * \param[in]  str  '\0' terminated string
 */
void oled_terminal(const char *str)
{
    char offset = _font[1];

    _y = OLED_HEIGHT - offset - 1;

    uint32_t i = 0;

    while(str[i] != '\0')
    {
        if(str[i] == '\n')
        {
            // Move the previous characters up
            for(uint32_t yn = offset; yn < OLED_HEIGHT; yn++)
            {
                for(uint32_t xn = 0; xn < OLED_WIDTH; xn++)
                {
                    oled_pixel_t val = OLED_PIXEL_OFF;

                    if(_framebuffer[xn + (yn / 8) * OLED_WIDTH] & (1 << (yn % 8)))
                    {
                        val = OLED_PIXEL_ON;
                    }

                    oled_setpixel(xn, yn - offset, val);
                }
            }

            // Clear bottom
            for(uint32_t yn = OLED_HEIGHT - offset - 1; yn < OLED_HEIGHT; yn++)
            {
                for(uint32_t xn = 0; xn < OLED_WIDTH; xn++)
                {
                    oled_setpixel(xn, yn, OLED_PIXEL_OFF);
                }
            }

            oled_goto(0, OLED_HEIGHT - offset - 1);
        }
        else if(str[i] == '\r')
        {
            oled_goto(0, OLED_HEIGHT - offset - 1);
        }
        else
        {
            oled_putchar(str[i]);
        }

        i++;
    }

    // Enable to always update the display after a terminal write
    oled_update();
}

/*!
 * \brief Draws a line from (x0,y0) tot (x1,y1)
 *
 * Draws a line in the frame buffer using Bresenham's line algorithm. The
 * implementation is based on the following paper:
 * https://csustan.csustan.edu/~tom/Lecture-Notes/Graphics/Bresenham-Line/Bresenham-Line.pdf
 *
 * Call the function oled_update() to actually show the result.
 *
 * \param[in]  x0  x-value of the start point
 * \param[in]  y0  y-value of the start point
 * \param[in]  x1  x-value of the end point
 * \param[in]  y1  y-value of the end point
 */
void oled_drawline(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{
    int dx = x1 - x0;
    int dy = y1 - y0;

    int stepx, stepy;

    if (dy < 0) { dy = -dy; stepy = -1; } else { stepy = 1; }
    if (dx < 0) { dx = -dx; stepx = -1; } else { stepx = 1; }

    dy <<= 1;
    dx <<= 1;

    if((x0 < OLED_WIDTH) && (y0 < OLED_HEIGHT))
    {
        oled_setpixel(x0, y0, OLED_PIXEL_ON);
    }

    if (dx > dy)
    {
        int fraction = dy - (dx >> 1);

        while (x0 != x1)
        {
            x0 += stepx;
            if (fraction >= 0)
            {
                y0 += stepy;
                fraction -= dx;
            }

            fraction += dy;
            if((x0 < OLED_WIDTH) && (y0 < OLED_HEIGHT))
            {
                oled_setpixel(x0, y0, OLED_PIXEL_ON);
            }
        }

    }
    else
    {
        int fraction = dx - (dy >> 1);

        while (y0 != y1)
        {
            if (fraction >= 0)
            {
                x0 += stepx;
                fraction -= dy;
            }
            y0 += stepy;

            fraction += dx;
            if((x0 < OLED_WIDTH) && (y0 < OLED_HEIGHT))
            {
                oled_setpixel(x0, y0, OLED_PIXEL_ON);
            }
        }
    }
}

/*!
 * \brief Draws a bitmap
 *
 * Copies a bitmap to the frame buffer.
 * Call the function oled_update() to actually display the result.
 * Bitmaps should be located in the files bitmaps.c and bitmaps.h.
 *
 * \param[in]  bitmap  A pointer to a bitmap
 */
void oled_drawbitmap(const unsigned char *bitmap)
{
    memcpy(_framebuffer, bitmap, sizeof(_framebuffer));
}

/*!
 * \brief Draws a frame of an animation
 *
 * Copies a frame of the animation to the framebuffer.
 * Call the function oled_update() to actually show the result.
 * Animations should be located in the files animations.c and animations.h.
 *
 * \param[in]  x      x-value of the start point
 * \param[in]  y      y-value of the start point
 * \param[in]  frame  A pointer to a frame of an animation
 * \param[in]  w      Width of the animation
 * \param[in]  h      Height of the animation
 */
void oled_drawanimation(uint16_t x, uint16_t y, const unsigned char *frame, uint8_t w, uint8_t h)
{
    unsigned char data = 0;

    for(int32_t j=0; j<w; j++)
    {
        for(int32_t i=0; i<h; i++)
        {
            if((i & 7) == 0)
            {
                data = frame[(j * w/8) + (i / 8)];
            }
            else
            {
                data <<= 1;
            }

            oled_setpixel(x+i, y+j, ((data & 0x80)) != 0 ? OLED_PIXEL_ON : OLED_PIXEL_OFF);
        }
    }
}
