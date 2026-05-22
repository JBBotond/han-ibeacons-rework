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
#include <stddef.h>
#include <string.h>

#include "lcd_par_s035.h"
#include "lpspi0_master.h"

// -----------------------------------------------------------------------------
// Local type definitions
// -----------------------------------------------------------------------------
#define ST7796S_CMD_SWRESET (0x01U)
#define ST7796S_CMD_SLPIN   (0x10U)
#define ST7796S_CMD_SLPOUT  (0x11U)
#define ST7796S_CMD_INVOFF  (0x20U)
#define ST7796S_CMD_INVON   (0x21U)
#define ST7796S_CMD_DISPOFF (0x28U)
#define ST7796S_CMD_DISPON  (0x29U)
#define ST7796S_CMD_CASET   (0x2AU)
#define ST7796S_CMD_RASET   (0x2BU)
#define ST7796S_CMD_RAMWR   (0x2CU)
#define ST7796S_CMD_TEOFF   (0x34U)
#define ST7796S_CMD_TEON    (0x35U)
#define ST7796S_CMD_MADCTL  (0x36U)
#define ST7796S_CMD_COLMOD  (0x3AU)
#define ST7796S_CMD_CSCON   (0xF0U)
#define ST7796S_CMD_INVTR   (0xB4U)
#define ST7796S_CMD_FRMCTR1 (0xB1U)
#define ST7796S_CMD_BPC     (0xB5U)
#define ST7796S_CMD_DFC     (0xB6U)
#define ST7796S_CMD_PWR1    (0xC0U)
#define ST7796S_CMD_PWR2    (0xC1U)
#define ST7796S_CMD_PWR3    (0xC2U)
#define ST7796S_CMD_VCMPCTL (0xC5U)
#define ST7796S_CMD_DOCA    (0xE8U)
#define ST7796S_CMD_PGC     (0xE0U)
#define ST7796S_CMD_NGC     (0xE1U)

// -----------------------------------------------------------------------------
// Local function prototypes
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Local variables
// -----------------------------------------------------------------------------
extern volatile uint32_t ms;

uint8_t lcd_buffer[32 * 32 * BYTES_PER_PIXEL] = {0};

/*
 * This is the panel-specific initialization parameters for driver section,
 * including analog power/voltage control, gamma correction, scan timing, etc.
 * Data format: | 1 Byte parameter length | 1 Byte command byte | N Bytes command parameter |
 */
static const uint8_t st7796s_driver_preset_par_s035[] = {
    0x01, ST7796S_CMD_CSCON,   0xC3,                   // Enable command part 1
    0x01, ST7796S_CMD_CSCON,   0x96,                   // Enable command part 2
    0x01, ST7796S_CMD_INVTR,   0x01,                   // Display inversion
    0x02, ST7796S_CMD_FRMCTR1, 0x80, 0x10,             // Frame rate control 1
    0x04, ST7796S_CMD_BPC,     0x1F, 0x50, 0x00, 0x20, // Blanking porch control
    0x03, ST7796S_CMD_DFC,     0x8A, 0x07, 0x3B,       // Display function control
    0x02, ST7796S_CMD_PWR1,    0x80, 0x64,             // Power control 1
    0x01, ST7796S_CMD_PWR2,    0x13,                   // Power control 2
    0x01, ST7796S_CMD_PWR3,    0xA7,                   // Power control 3
    0x01, ST7796S_CMD_VCMPCTL, 0x09,                   // VCOM control
    0x08, ST7796S_CMD_DOCA,    0x40, 0x8A, 0x00, 0x00, // DOCA
                               0x29, 0x19, 0xA5, 0x33,
    0x0E, ST7796S_CMD_PGC,     0xF0, 0x06, 0x0B, 0x07, // PGC
                               0x06, 0x05, 0x2E, 0x33,
                               0x47, 0x3A, 0x17, 0x16,
                               0x2E, 0x31,
    0x0E, ST7796S_CMD_NGC,     0xF0, 0x09, 0x0D, 0x09, // NGC
                               0x08, 0x23, 0x2E, 0x33,
                               0x46, 0x38, 0x13, 0x13,
                               0x2C, 0x32,
    0x01, ST7796S_CMD_CSCON,   0x3C,                   // Disable command part 1
    0x01, ST7796S_CMD_CSCON,   0x69,                   // Disable command part 2
};

// -----------------------------------------------------------------------------
// Local function implementation
// -----------------------------------------------------------------------------
void lcd_init(void)
{
    lpspi0_master_init();

    lcd_software_reset();

    // Send preset commands
    uint32_t total_len = sizeof(st7796s_driver_preset_par_s035);
    const uint8_t *p = st7796s_driver_preset_par_s035;

    while(total_len > 0)
    {
        uint8_t param_len = p[0];
        uint8_t command   = p[1];
        const uint8_t *params = &p[2];

        lpspi0_master_tx_cmd(&command, 1);

        if (param_len > 0)
        {
            lpspi0_master_tx_data(params, param_len);
        }

        p += (param_len + 2);
        total_len -= (param_len + 2);
    }

    // Set color mode based on BYTES_PER_PIXEL
    #if (BYTES_PER_PIXEL == 3U)
    lcd_set_pixel_format(LCD_PixelFormatRGB888);
    #elif (BYTES_PER_PIXEL == 2U)
    lcd_set_pixel_format(LCD_PixelFormatRGB565);
    #else
    #error "Unsupported BYTES_PER_PIXEL value"
    #endif

    // Set invert display
    lcd_set_invert_display(true);

    // Flip horizontally
    lcd_flip(true, false);

    // Exit sleep mode
    lcd_sleep_mode(false);

    // Enable display
    lcd_display_enable(true);

    // Clear lcd
    lcd_clear();
}

void lcd_select_area(uint16_t startX, uint16_t startY, uint16_t endX, uint16_t endY)
{
    uint8_t tx_buf[4];

    tx_buf[0] = ST7796S_CMD_CASET;
    lpspi0_master_tx_cmd(tx_buf, 1);

    tx_buf[0] = (uint8_t)(startX >> 0x08U) & 0xFFU;
    tx_buf[1] = (uint8_t)startX & 0xFFU;
    tx_buf[2] = (uint8_t)(endX >> 0x08U) & 0xFFU;
    tx_buf[3] = (uint8_t)endX & 0xFFU;

    lpspi0_master_tx_data(tx_buf, 4U);


    tx_buf[0] = ST7796S_CMD_RASET;
    lpspi0_master_tx_cmd(tx_buf, 1);

    tx_buf[0] = (uint8_t)(startY >> 0x08U) & 0xFFU;
    tx_buf[1] = (uint8_t)startY & 0xFFU;
    tx_buf[2] = (uint8_t)(endY >> 0x08U) & 0xFFU;
    tx_buf[3] = (uint8_t)endY & 0xFFU;

    lpspi0_master_tx_data(tx_buf, 4U);
}

void lcd_write_pixels(const uint8_t *pixels, uint32_t length)
{
    uint8_t cmd = ST7796S_CMD_RAMWR;
    lpspi0_master_tx_cmd(&cmd, 1);
    lpspi0_master_tx_data(pixels, length);
}

void lcd_set_pixel_format(lcd_pixel_format_t pixelFormat)
{
    uint8_t cmd = ST7796S_CMD_COLMOD;
    lpspi0_master_tx_cmd(&cmd, 1);
    lpspi0_master_tx_data(&pixelFormat, 1);
}

void lcd_software_reset(void)
{
    uint8_t cmd = ST7796S_CMD_SWRESET;
    lpspi0_master_tx_cmd(&cmd, 1);

    // Delay 5 ms
    uint32_t start = ms;
    while((ms - start) < 5)
    {}
}

void lcd_set_orientation(lcd_orientation_mode_t orientationMode)
{
    uint8_t cmd = ST7796S_CMD_MADCTL;
    lpspi0_master_tx_cmd(&cmd, 1);
    lpspi0_master_tx_data((const uint8_t *)&orientationMode, 1);
}

void lcd_flip(bool horizontal, bool vertical)
{
    uint8_t madctl = 0x00U;

    if (horizontal)
    {
        madctl |= 0x40U; // MX bit
    }

    if (vertical)
    {
        madctl |= 0x80U; // MY bit
    }

    uint8_t cmd = ST7796S_CMD_MADCTL;
    lpspi0_master_tx_cmd(&cmd, 1);
    lpspi0_master_tx_data(&madctl, 1);
}

void lcd_set_invert_display(bool invert)
{
    uint8_t cmd;

    if (invert)
    {
        cmd = ST7796S_CMD_INVON;
    }
    else
    {
        cmd = ST7796S_CMD_INVOFF;
    }

    lpspi0_master_tx_cmd(&cmd, 1);
}

void lcd_sleep_mode(bool sleep)
{
    uint8_t cmd;

    if (sleep)
    {
        cmd = ST7796S_CMD_SLPIN;
    }
    else
    {
        cmd = ST7796S_CMD_SLPOUT;
    }

    lpspi0_master_tx_cmd(&cmd, 1);
}

void lcd_display_enable(bool enable)
{
    uint8_t cmd;

    if (enable)
    {
        cmd = ST7796S_CMD_DISPON;
    }
    else
    {
        cmd = ST7796S_CMD_DISPOFF;
    }

    lpspi0_master_tx_cmd(&cmd, 1);
}

void lcd_clear(void)
{
    memset(lcd_buffer, 0x00, sizeof(lcd_buffer));

    for(uint32_t b=0; b<15; b++)
    {
        for(uint32_t a=0; a<10; a++)
        {
            lcd_select_area(a*32, b*32, a*32+31, b*32+31);
            lcd_write_pixels(lcd_buffer, sizeof(lcd_buffer));
        }
    }
}
