/*! ***************************************************************************
 *
 * \brief     TFT LCD (ILI9341 and XPT2046) driver
 * \file      tft_lcd.c
 * \author    Hugo Arends
 * \date      March 2026
 *
 * \ref       https://www.tinytronics.nl/en/displays/tft/2.8-inch-tft-display-240*320-pixels-with-touchscreen-spi-ili9341
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
#include "tft_lcd.h"
#include "lpspi_master.h"

#include <string.h>

// This driver assumes that ms exists and is incremented every 1 ms
extern volatile uint32_t ms;

/*!
 * \brief Frame buffer
 *
 * The frame buffer holds all the data. The frame buffer is only written to the
 * TFT LCD when the function lcd_write_pixels() is called.
 */
uint16_t lcd_framebuffer[LCD_FRAMEBUFFER_WIDTH * LCD_FRAMEBUFFER_HEIGHT];

/*!
 * \brief Width and height, will be updated on orientation change
 */
int16_t lcd_width = LCD_WIDTH;
int16_t lcd_height = LCD_HEIGHT;

/*!
 * \brief (x,y) coordinates of the last detected touch
 */
int16_t lcd_touch_x = 0;
int16_t lcd_touch_y = 0;
int16_t lcd_touch_z = 0;

/*!
 * \brief Local orientation value
 */
static orientation_t _orientation = ORIENTATION_0;

/*!
 * \brief Pointer to the selected font
 *
 * The available fonts are in fonts.h and fonts.c.
 *
 * Note that Monospaced_plain_10 is the default font and should not be removed
 * from fonts.c and fonts.h.
 */
static const char *_font = NULL;

/*!
 * \brief List of commands that will be send to the TFT LCD upon
 *        initialisation
 *
 * The list of commands is constructed as follows:
 *  [param_len],[command],
 *  [param_1],[param_2],...,[param_n],
 */
static const uint8_t _lcd_init_commands[] =
{
    5, 0xCB, // Power control A
    0x39, 0x2C, 0x00, 0x34, 0x02,

    3, 0xCF, // Power control B
    0x00, 0x83, 0x30,

    3, 0xE8, // Driver timing control A
    0x85, 0x01, 0x79,

    2, 0xEA, // Driver timing control B
    0x00, 0x00,

    4, 0xED, // Power on sequence control
    0x64, 0x03, 0x12,0x81,

    1, 0xF7, // Pump ratio control
    0x20,

    1, 0xC0, // Power control 1
    0x26,

    1, 0xC1, // Power control 2
    0x11,

    2, 0xC5, // VCOM control 1
    0x35, 0x3E,

    1, 0xC7, // VCOM control 2
    0xBE,

    1, 0x36, // Memory Access Control
    0x28,

    1, 0x3A, // COLMOD: Pixel Format Set
    0x55,

    2, 0xB1, // Frame Rate Control (In Normal Mode/Full Colors)
    0x00, 0x1F,

    4, 0xB6, // Display Function Control
    0x0A, 0x82, 0x27, 0x00,

    1, 0xF2, // Enable 3G
    0x00,

    4, 0x2A, // Column Address Set
    0x00, 0x00, 0x00, 0xEF,

    4, 0x2B, // Page Address Set
    0x00, 0x00, 0x01, 0x3F,

    1, 0x26, // Gamma Set
    0x01,

    15, 0xE0, // Positive Gamma Correction
    0x0F, 0x31, 0x2B, 0x0C, 0x0E,
    0x08, 0x4E, 0xF1, 0x37, 0x07,
    0x10, 0x03, 0x0E, 0x09, 0x00,

    15, 0xE1, // Negative Gamma Correction
    0x00, 0x0E, 0x14, 0x03, 0x11,
    0x07, 0x31, 0xC1, 0x48, 0x08,
    0x0F, 0x0C, 0x31, 0x36, 0x0F,
};

/*!
 * \brief Sends a sequence of initialisation commands to the TFT LCD
 *
 * Refer to the ILI9341 datasheet for a description of all commands
 */
void lcd_init(void)
{
    uint32_t start_time;
    uint8_t command;

    // Clear the frame buffer
    for(uint32_t i=0; i<(LCD_FRAMEBUFFER_WIDTH * LCD_FRAMEBUFFER_HEIGHT); ++i)
    {
        lcd_framebuffer[i] = 0x0000;
    }

    // Make sure to wait for 200 ms after power on
    while(ms < 200)
    {}

    // Initialize the SPI module and all other pins
    // To maintain a good overview of all pins in used for the module, all
    // pins are initialized in the following function.
    lpspi_master_init();

    // Initialize display controller
    lpspi_set_prescale(0);
    lpspi_set_pcs(LPSPI_PCS1);

    // Software Reset
    command = 0x01;
    lpspi_transmit_command(&command, 1UL);

    // Wait for 10 ms
    start_time = ms;
    while(ms - start_time < 10)
    {}

    // Display OFF
    command = 0x28; //This command is specified in the display's datasheet.
    lpspi_transmit_command(&command, 1UL);

    uint32_t total_len = sizeof(_lcd_init_commands);
    uint8_t *p = (uint8_t *)_lcd_init_commands;

    while(total_len > 0)
    {
        uint8_t param_len = p[0];//5
        uint8_t command   = p[1];//0xCB
        uint8_t *params   = &p[2];//the address of 0x39

        // Trasmit command
        lpspi_transmit_command(&command, 1UL);

        // Transmit parameters if there are any
        if (param_len > 0)
        {
            lpspi_transmit_data(params, param_len);
        }

        p += (param_len + 2);
        total_len -= (param_len + 2);
    }

    // Sleep Out
    command = 0x11;
    lpspi_transmit_command(&command, 1UL);

    // Wait for 120 ms
    start_time = ms;
    while(ms - start_time < 120)
    {}

    // Display ON
    command = 0x29;
    lpspi_transmit_command(&command, 1UL);

    // Clear the display
    lcd_clear(RGB_BLACK);

    // Backlight on
    lcd_backlight(true);

    // Default orientation, also selects BGR mode
    lcd_orientation(0);
}

/*!
 * \brief Turns the backlight on or off
 *
 * The backlight is controlled by GPIO2 pin 2. When the pin is set to 1, the
 * backlight is on. When the pin is set to 0, the backlight is off.
 *
 * \param on true to turn the backlight on, false to turn it off
 */
void lcd_backlight(bool on)
{
    if(on)
    {
        GPIO2->PSOR = (1<<2);
    }
    else
    {
        GPIO2->PCOR = (1<<2);
    }
}

/*!
 * \brief Sets the display to inverse mode
 *
 * \param on true to turn on inverse mode, false to turn it off
 */
void lcd_inverse(bool on)
{
    uint8_t cmd;

    if (on)
    {
        // Display Inversion ON
        cmd = 0x21;
    }
    else
    {
        // Display Inversion OFF
        cmd = 0x20;
    }

    lpspi_set_prescale(0);
    lpspi_set_pcs(LPSPI_PCS1);

    lpspi_transmit_command(&cmd, 1UL);
}

/*!
 * \brief Sets the orientation of the display
 *
 * The orientation can be set to 0, 90, 180 or 270 degrees. The orientation is
 * set by writing to the MADCTL register. The MADCTL register controls the order
 * of the pixels and also the RGB/BGR order.
 *
 * \param orientation See orientation_t for possible values
 */
void lcd_orientation(orientation_t orientation)
{
    _orientation = orientation;

    // Memory Access Control
    uint8_t command = 0x36;
    uint8_t data;

    switch(orientation)
    {
        case ORIENTATION_0:
            // MY = [0]: Bottom to top (Row Address Order)
            // MX = [1]: Right to left (Column Address Order)
            // MV = [0]: Normal mode (Row / Column Exchange)
            // BGR = [1]: BGR mode (RGB-BGR Order)
            data = (0x40 | 0x08);

            lcd_width = LCD_WIDTH;
            lcd_height = LCD_HEIGHT;

            break;
        case ORIENTATION_90:
            // MY = [0]: Bottom to top (Row Address Order)
            // MX = [0]: Left to right (Column Address Order)
            // MV = [1]: Reverse mode (Row / Column Exchange)
            // BGR = [1]: BGR mode (RGB-BGR Order)
            data = (0x20 | 0x08);

            lcd_width = LCD_HEIGHT;
            lcd_height = LCD_WIDTH;

            break;
        case ORIENTATION_180:
            // MY = [1]: Top to bottom (Row Address Order)
            // MX = [0]: Right to left (Column Address Order)
            // MV = [0]: Normal mode (Row / Column Exchange)
            // BGR = [1]: BGR mode (RGB-BGR Order)
            data = (0x80 | 0x08);

            lcd_width = LCD_WIDTH;
            lcd_height = LCD_HEIGHT;

            break;
        case ORIENTATION_270:
        default:
            // MY = [1]: Top to bottom (Row Address Order)
            // MX = [1]: Left to right (Column Address Order)
            // MV = [1]: Reverse mode (Row / Column Exchange)
            // BGR = [1]: BGR mode (RGB-BGR Order)
            data = (0xE0 | 0x08);

            lcd_width = LCD_HEIGHT;
            lcd_height = LCD_WIDTH;

            break;
    }

    lpspi_set_prescale(0);
    lpspi_set_pcs(LPSPI_PCS1);

    lpspi_transmit_command(&command, 1UL);
    lpspi_transmit_data(&data, 1UL);
}

/*!
 * \brief Sets the area of the display to be updated
 *
 * \param start_x  The x-coordinate of the top-left corner of the area
 * \param start_y  The y-coordinate of the top-left corner of the area
 * \param width  The width of the area
 * \param height  The height of the area
 */
void lcd_set_area(uint16_t start_x, uint16_t start_y, uint16_t width, uint16_t height)
{
    uint8_t command;
    uint8_t data[4];

    // Column Address Set (2Ah)
    command = 0x2A;
    data[0] = (uint8_t)(start_x >> 0x08U) & 0xFFU;
    data[1] = (uint8_t)start_x & 0xFFU;
    data[2] = (uint8_t)((start_x + width - 1) >> 0x08U) & 0xFFU;
    data[3] = (uint8_t)(start_x + width - 1) & 0xFFU;

    lpspi_set_prescale(0);
    lpspi_set_pcs(LPSPI_PCS1);

    lpspi_transmit_command(&command, 1UL);
    lpspi_transmit_data(data, 4UL);

    // Page Address Set (2Bh)
    command = 0x2B;
    data[0] = (uint8_t)(start_y >> 0x08U) & 0xFFU;
    data[1] = (uint8_t)start_y & 0xFFU;
    data[2] = (uint8_t)((start_y + height - 1) >> 0x08U) & 0xFFU;
    data[3] = (uint8_t)(start_y + height - 1) & 0xFFU;

    lpspi_transmit_command(&command, 1UL);
    lpspi_transmit_data(data, 4UL);
}

/*!
 * \brief Writes pixels to the TFT LCD
 *
 * Pixel must be of type BGR565
 * Use the lcd_set_area() function to set the area that will be written
 *
 * \param pixels Pointer to the array of pixels to write
 * \param length Number of pixels to write
 */
void lcd_write_pixels(uint16_t *pixels, uint32_t length)
{
    // Memory Write (2Ch)
    uint8_t command = 0x2C;

    lpspi_set_prescale(0);
    lpspi_set_pcs(LPSPI_PCS1);

    lpspi_transmit_command(&command, 1);
    lpspi_transmit_data((uint8_t *)pixels, length * 2);
}

/*!
 * \brief Clears the TFT LCD
 *
 * The function clears the frame buffer and then writes the frame buffer to the
 * TFT LCD. The function divides the display into areas of the same size as
 * the frame buffer and writes the frame buffer to each area. This is done
 * because the microcontroller has limited RAM and cannot hold a full frame
 * buffer for the entire display.
 */
void lcd_clear(uint16_t color)
{
    for(uint32_t i=0; i<(LCD_FRAMEBUFFER_HEIGHT * LCD_FRAMEBUFFER_WIDTH); i++)
    {
        lcd_framebuffer[i] = color;
    }

    for(uint32_t h=0; h<(lcd_height / LCD_FRAMEBUFFER_HEIGHT); h++)
    {
        for(uint32_t w=0; w<(lcd_width / LCD_FRAMEBUFFER_WIDTH); w++)
        {
            lcd_set_area(
                (w * LCD_FRAMEBUFFER_WIDTH),
                (h * LCD_FRAMEBUFFER_HEIGHT),
                LCD_FRAMEBUFFER_WIDTH,
                LCD_FRAMEBUFFER_HEIGHT);

            lcd_write_pixels(lcd_framebuffer, LCD_FRAMEBUFFER_HEIGHT * LCD_FRAMEBUFFER_WIDTH);
        }
    }
}

/*! \brief Gets the touch coordinates from the XPT2046 touch controller
 *
 * The touch coordinates are stored in the global variables lcd_touch_x and
 * lcd_touch_y. The touch coordinates are updated every time this function is
 * called. The function should be called when a touch is detected, for
 * example in the GPIO2 interrupt handler.
 */
void lcd_get_touch(void)
{
    // "It is recommended that the processor mask the interrupt
    //  PENIRQ is associated with whenever the processor sends a
    //  control byte to the XPT2046. This prevents false triggering
    //  of interrupts when the PENIRQ output is disabled in the
    //  cases discussed in this section."

    // Disable GPIO2 interrupts
    GPIO2->ICR[3] = GPIO_ICR_ISF(1);

    const uint8_t tx_data[] =
    {
        0xB1, // Z1
        0x00, // dummy
        0xC1, // Z2
        0x00, // dummy
        0x91, // X (dummy)
        0x00, // dummy
        0xD1, // Y (dummy)
        0x00, // dummy
        0x91, // X
        0x00, // dummy
        0xD1, // Y
        0x00, // dummy
        0x91, // X
        0x00, // dummy
        0xD1, // Y
        0x00, // dummy
        0x91, // X
        0x00, // dummy
        0xD1, // Y
        0x00, // dummy
        0x91, // X
        0x00, // dummy
        0xD0, // Y
        0x00, // dummy
        0x00, // dummy
    };

    uint8_t rx_data[sizeof(tx_data)] = {0};

    // Transfer data
    lpspi_set_prescale(7);
    lpspi_set_pcs(LPSPI_PCS0);

    lpspi_transceive_data((uint8_t *)tx_data, rx_data, sizeof(tx_data));

    // rx_data[0] is dummy data (not a response to a command)

    int16_t z1 = (rx_data[1] << 5) | (rx_data[2] >> 3);
    int16_t z2 = (rx_data[3] << 5) | (rx_data[4] >> 3);
    int16_t z = z1 + 4095 - z2;

    uint16_t x = ((rx_data[9]  << 5) | (rx_data[10] >> 3)) +
                 ((rx_data[13]  << 5) | (rx_data[14] >> 3)) +
                 ((rx_data[17]  << 5) | (rx_data[18] >> 3)) +
                 ((rx_data[21]  << 5) | (rx_data[22] >> 3));
    uint16_t y = ((rx_data[11] << 5) | (rx_data[12] >> 3)) +
                 ((rx_data[15] << 5) | (rx_data[16] >> 3)) +
                 ((rx_data[19] << 5) | (rx_data[20] >> 3)) +
                 ((rx_data[23] << 5) | (rx_data[24] >> 3));

    if(z >= 300)
    {
        // Calculate average of the measurements for x and y to improve accuracy
        lcd_touch_x = x >> 2;
        lcd_touch_y = y >> 2;
        lcd_touch_z = z;

        // Adjust the touch coordinates based on the current orientation of the
        // display
        //
        //       Touch        ORIENTATION     ORIENTATION     ORIENTATION     ORIENTATION
        //    coordinates          0               90             180             270
        //     -> Y            -> X                 Y <-            Y <-        -> Y
        //    +---------+     +---------+     +---------+     +---------+     +---------+
        //  | |         |   | |         |   | |         |     |         |     |         |
        //  V |         |   V |         |   V |         |     |         |     |         |
        //  X |         |   Y |         |   X |         |   X |         |   X |         |
        //    |         |     |         |     |         |   ^ |         |   ^ |         |
        //    |         |     |         |     |         |   | |         |   | |         |
        //    +---------+     +---------+     +---------+     +---------+     +---------+

        int16_t tmp;

        switch (_orientation)
        {
            case ORIENTATION_0:
                tmp = lcd_touch_x;
                lcd_touch_x = lcd_touch_y;
                lcd_touch_y = tmp;
            break;
            case ORIENTATION_90:
                lcd_touch_y = 4095 - lcd_touch_y;
            break;
            case ORIENTATION_180:
                tmp = lcd_touch_x;
                lcd_touch_x = 4095 - lcd_touch_y;
                lcd_touch_y = 4095 - tmp;
            break;
            case ORIENTATION_270:
            default:
                lcd_touch_x = 4095 - lcd_touch_x;
            break;
        }

        // Map the touch coordinates to the TFT LCD coordinates
        // lcd_touch_x = (uint16_t)((float)lcd_touch_x / 4095.0f * lcd_width);
        // lcd_touch_y = (uint16_t)((float)lcd_touch_y / 4095.0f * lcd_height);
    }

    // Enable GPIO2 interrupts
    GPIO2->ICR[3] = GPIO_ICR_ISF(1) | GPIO_ICR_IRQC(0b1010);
}

/*!
 * \brief Sets the font
 *
 * Changing a font doesn't change what is already on the display.
 * This font will be used when writing new characters.
 *
 * Fonts should be located in the files fonts.c and fonts.h.
 *
 * \param[in]  f  A pointer to a font
 */
void lcd_set_font(const char *f)
{
    _font = f;
}

/*!
 * \brief Writes a character to the TFT LCD
 *
 * The character is written to the TFT LCD at the specified (x,y) coordinates. The
 * (x,y) coordinates specify the top-left corner of the character. The text_color
 * parameter specifies the color of the character, and the bg_color parameter
 * specifies the color of the background. The function returns the width of the
 * character in pixels, which can be used to calculate the x coordinate for the
 * next character when writing a string of characters.
 *
 * \param[in]  x           The x-coordinate of the top-left corner of the character
 * \param[in]  y           The y-coordinate of the top-left corner of the character
 * \param[in]  c           The character to write
 * \param[in]  text_color  The color of the character in BGR565 format
 * \param[in]  bg_color    The color of the background in BGR565 format
 *
 * \return The width of the character in pixels
 */
uint16_t lcd_put_char(uint16_t x, uint16_t y, char c, uint16_t text_color, uint16_t bg_color)
{
    // Get the first four parameters from the font
    // uint8_t font_width = _font[0];
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

    // Check buffer boundaries
    if((char_width * font_height) > (LCD_FRAMEBUFFER_WIDTH * LCD_FRAMEBUFFER_HEIGHT))
    {
        // The font does not match the size of the frame buffer, so it cannot
        // be displayed
        return 0;
    }

    // Check LCD size boundaries
    if((x + char_width) >= lcd_width)
    {
        // The character is too wide to be displayed at the given x coordinate
        return 0;
    }

    if((y + font_height) >= lcd_height)
    {
        // The character is too high to be displayed at the given y coordinate
        return 0;
    }

    // Calculate the index in the data table
    index = 4 + (font_numchars * 4) + (256UL * offset1) + offset2;

    // Calculate the number of bytes for each column
    uint8_t bytes_per_col = (font_height / 8);
    if((font_height % 8) > 0)
    {
        bytes_per_col++;
    }

    uint32_t framebuffer_i = 0;

    // Loop all pixels in a character
    for(uint32_t h = 0; h < font_height; h++)
    {
        // Set initial byte offset, depending on the height of the font.
        uint32_t n = h / 8;

        for(uint32_t w = 0; w < char_width; w++)
        {
            uint8_t data = 0x00;

            // The font table stores pixel data for each character from left-top
            // to right bottom. if bytes are omitted in the font table, it means
            // that the remaining pixels in the column are 0 (off).
            if(n < n_bytes)
            {
                // Get the byte for this column and row
                data = _font[index + n];
            }

            // Set the mask for the current row, depending on the height
            // of the font.
            uint8_t mask = 0x01 << (h % 8);

            if((data & mask) > 0)
            {
                // Set the pixel in the frame buffer
                lcd_framebuffer[framebuffer_i++] = text_color;
            }
            else
            {
                // Set the pixel in the frame buffer to black
                lcd_framebuffer[framebuffer_i++] = bg_color;
            }

            n += bytes_per_col;
        }
    }

    // Set the area of the display to be updated
    lcd_set_area(x, y, char_width, font_height);

    // Write the pixels to the display
    lcd_write_pixels(lcd_framebuffer, char_width * font_height);

    return char_width;
}

void lcd_put_string(uint16_t x, uint16_t y, const char *str, uint16_t text_color, uint16_t bg_color)
{
    while(*str != '\0')
    {
        x += lcd_put_char(x, y, *str, text_color, bg_color);
        str++;
    }
}

/*!
 * \brief Draws a bitmap
 *
 * Writes a bitmap to the TFT LCD. The bitmap must be in BGR565 format and must
 * be stored in an array of uint16_t. The bitmap will be drawn at the specified
 * (x,y) coordinates. The (x,y) coordinates specify the top-left corner of the
 * bitmap. The width and height of the bitmap must be specified. The function
 * does not check if the bitmap fits within the display boundaries, so it is the
 * responsibility of the caller to ensure that the bitmap is drawn within the
 * display boundaries. The function also does not check if the bitmap is in the
 * correct format, so it is the responsibility of the caller to ensure that the
 * bitmap is in the correct format.
 *
 * Bitmaps should be located in the files bitmaps.c and bitmaps.h.
 *
 * \param[in]  x       The x-coordinate of the top-left corner of the bitmap
 * \param[in]  y       The y-coordinate of the top-left corner of the bitmap
 * \param[in]  width   The width of the bitmap
 * \param[in]  height  The height of the bitmap
 * \param[in]  bitmap  A pointer to a bitmap
 */
void lcd_draw_bitmap(uint16_t x, uint16_t y, uint16_t width, uint16_t height, const uint16_t *bitmap)
{
    lcd_set_area(x, y, width, height);
    lcd_write_pixels((uint16_t *)bitmap, width * height);
}

/*!
 * \brief Draws a frame of an animation
 *
 * Animations should be located in the files animations.c and animations.h.
 *
 * \param[in]  x         x-value of the start point
 * \param[in]  y         y-value of the start point
 * \param[in]  w         Width of the animation
 * \param[in]  h         Height of the animation
 * \param[in]  frame     A pointer to a frame of an animation
 * \param[in]  color     The color of the animation in BGR565 format
 * \param[in]  bg_color  The color of the background in BGR565 format
 */
void lcd_draw_animation(uint16_t x, uint16_t y, uint8_t width, uint8_t height, const unsigned char *frame,  uint16_t color, uint16_t bg_color)
{
    unsigned char data = 0;

    uint32_t framebuffer_i = 0;

    for(int32_t w=0; w<width; w++)
    {
        for(int32_t h=0; h<height; h++)
        {
            if((h & 7) == 0)
            {
                data = frame[(w * width/8) + (h / 8)];
            }
            else
            {
                data <<= 1;
            }

            lcd_framebuffer[framebuffer_i++] = (data & 0x80) ? color : bg_color;
        }
    }

    lcd_set_area(x, y, width, height);
    lcd_write_pixels(lcd_framebuffer, width * height);
}
