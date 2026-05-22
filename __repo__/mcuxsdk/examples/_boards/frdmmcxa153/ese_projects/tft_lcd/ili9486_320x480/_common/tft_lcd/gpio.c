/*! ***************************************************************************
 *
 * \brief     Low level driver for GPIO communication with the TFT LCD
 * \file      gpio.c
 * \author    Hugo Arends
 * \date      March 2026
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
 ******************************************************************************/
#include "gpio.h"
#include <stdbool.h>
#include <stddef.h>

// -----------------------------------------------------------------------------
// Local type definitions
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Local function prototypes
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Local variables
// -----------------------------------------------------------------------------

// This driver assumes that ms exists and is incremented every 1 ms
extern volatile uint32_t ms;

// -----------------------------------------------------------------------------
// Local function implementation
// -----------------------------------------------------------------------------
void gpio_init(void)
{
    // Enable modules and leave others unchanged
    MRCC0->MRCC_GLB_CC0_SET = MRCC_MRCC_GLB_CC0_PORT1(1);
    MRCC0->MRCC_GLB_CC1_SET = MRCC_MRCC_GLB_CC1_GPIO1(1);
    MRCC0->MRCC_GLB_CC0_SET = MRCC_MRCC_GLB_CC0_PORT2(1);
    MRCC0->MRCC_GLB_CC1_SET = MRCC_MRCC_GLB_CC1_GPIO2(1);
    MRCC0->MRCC_GLB_CC1_SET = MRCC_MRCC_GLB_CC1_PORT3(1);
    MRCC0->MRCC_GLB_CC1_SET = MRCC_MRCC_GLB_CC1_GPIO3(1);

    // Release modules from reset and leave others unchanged
    MRCC0->MRCC_GLB_RST0_SET = MRCC_MRCC_GLB_RST0_PORT1(1);
    MRCC0->MRCC_GLB_RST1_SET = MRCC_MRCC_GLB_RST1_GPIO1(1);
    MRCC0->MRCC_GLB_RST0_SET = MRCC_MRCC_GLB_RST0_PORT2(1);
    MRCC0->MRCC_GLB_RST1_SET = MRCC_MRCC_GLB_RST1_GPIO2(1);
    MRCC0->MRCC_GLB_RST1_SET = MRCC_MRCC_GLB_RST1_PORT3(1);
    MRCC0->MRCC_GLB_RST1_SET = MRCC_MRCC_GLB_RST1_GPIO3(1);

    // Configure pins
    PORT2->PCR[0] = PORT_PCR_LK(1); // P2_0   - LCD_DB00
    PORT2->PCR[1] = PORT_PCR_LK(1); // P2_1   - LCD_DB01
    PORT2->PCR[2] = PORT_PCR_LK(1); // P2_2   - LCD_DB02
    PORT2->PCR[3] = PORT_PCR_LK(1); // P2_3   - LCD_DB03
    PORT2->PCR[4] = PORT_PCR_LK(1); // P2_4   - LCD_DB04
    PORT2->PCR[5] = PORT_PCR_LK(1); // P2_5   - LCD_DB05
    PORT2->PCR[6] = PORT_PCR_LK(1); // P2_6   - LCD_DB06
    PORT2->PCR[7] = PORT_PCR_LK(1); // P2_7   - LCD_DB07

    PORT1->PCR[4]  = PORT_PCR_LK(1); // P1_4  - LCD_DB08
    PORT1->PCR[5]  = PORT_PCR_LK(1); // P1_5  - LCD_DB09
    PORT1->PCR[6]  = PORT_PCR_LK(1); // P1_6  - LCD_DB10
    PORT1->PCR[7]  = PORT_PCR_LK(1); // P1_7  - LCD_DB11
    PORT1->PCR[8]  = PORT_PCR_LK(1); // P1_8  - LCD_DB12
    PORT1->PCR[9]  = PORT_PCR_LK(1); // P1_9  - LCD_DB13
    PORT1->PCR[10] = PORT_PCR_LK(1); // P1_10 - LCD_DB14
    PORT1->PCR[11] = PORT_PCR_LK(1); // P1_11 - LCD_DB15

    PORT3->PCR[6] = PORT_PCR_LK(1); // P3_6 - LCD_RS
    PORT3->PCR[7] = PORT_PCR_LK(1); // P3_7 - LCD_WR
    PORT3->PCR[8] = PORT_PCR_LK(1); // P3_8 - LCD_CS
    PORT3->PCR[9] = PORT_PCR_LK(1); // P3_9 - LCD_RST

                                    // See FatFS driver
                                    // P1_0 - SPI_MOSI
                                    // P1_1 - SPI_CLK
                                    // P1_2 - SPI_MISO
                                    // P1_3 - SD_CS

    // Set GPIO pins initial output values and directions
    GPIO1->PCOR  = (1<<4) | (1<<5) | (1<<6) | (1<<7) | (1<<8) | (1<<9) | (1<<10) | (1<<11);
    GPIO1->PDDR |= (1<<4) | (1<<5) | (1<<6) | (1<<7) | (1<<8) | (1<<9) | (1<<10) | (1<<11);

    GPIO2->PCOR  = (1<<0) | (1<<1) | (1<<2) | (1<<3) | (1<<4) | (1<<5) | (1<<6) | (1<<7);
    GPIO2->PDDR |= (1<<0) | (1<<1) | (1<<2) | (1<<3) | (1<<4) | (1<<5) | (1<<6) | (1<<7);

    // LCD_RS = [0] : 0 = command, 1 = data
    // LCD_WR = [1] : Write signal on rising edge
    // LCD_CS = [1] : LCD is not selected
    // LCD_RST = [0] : LCD is in reset
    GPIO3->PCOR  = (1<<6) | (1<<9);
    GPIO3->PSOR  = (1<<7) | (1<<8);
    GPIO3->PDDR |= (1<<6) | (1<<7) | (1<<8) | (1<<9);

    // Delay for a short time to ensure that the LCD is properly reset
    uint32_t start_time = ms;
    while(ms - start_time < 10)
    {}

    // Release LCD from reset
    GPIO3->PSOR = (1<<9);
}

void gpio_transmit(uint16_t *tx_buffer, const uint32_t n)
{
    // Set GPIO pins to output
    GPIO1->PDDR |= (1<<4) | (1<<5) | (1<<6) | (1<<7) | (1<<8) | (1<<9) | (1<<10) | (1<<11);
    GPIO2->PDDR |= (1<<0) | (1<<1) | (1<<2) | (1<<3) | (1<<4) | (1<<5) | (1<<6) | (1<<7);

    // CS = [0] : LCD is selected
    GPIO3->PCOR = (1<<8);

    for(uint32_t i = 0; i < n; ++i)
    {
        // Prepare write pulse (set low)
        GPIO3->PCOR = (1<<7);

        uint16_t lsb = tx_buffer[i] & 0x00FF;
        uint16_t msb = (tx_buffer[i] >> 4) & 0x0FF0;

        // Write data to the data bus
        GPIO2->PDOR = (GPIO2->PDOR & 0xFFFFFF00) | lsb;
        GPIO1->PDOR = (GPIO1->PDOR & 0xFFFFF00F) | msb;

        // Generate a write pulse (rising edge)
        GPIO3->PSOR = (1<<7);
    }

    // CS = [1] : LCD is not selected
    GPIO3->PSOR = (1<<8);
}

void gpio_transmit_command(uint16_t *tx_buffer, const uint32_t n)
{
    // RS = [0] : command
    GPIO3->PCOR = (1<<6);

    // Transmit data
    gpio_transmit(tx_buffer, n);
}

void gpio_transmit_data(uint16_t *tx_buffer, const uint32_t n)
{
    // RS = [1] : data
    GPIO3->PSOR = (1<<6);

    // Transmit data
    gpio_transmit(tx_buffer, n);
}
