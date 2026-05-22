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
#include <stdbool.h>
#include <stdio.h>

#include "leds.h"
#include "serial.h"

#include "crc.h"

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
void print_result(uint32_t data[], size_t data_len, uint32_t expected_checksum,
    uint32_t calculated_checksum);

// -----------------------------------------------------------------------------
// Local variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Main application
// -----------------------------------------------------------------------------
int main(void)
{
    leds_init();
    crc_init();
    serial_init(115200);

    printf("CRC - CRC32 example\r\n");
    printf("%s build %s %s\r\n", TARGETSTR, __DATE__, __TIME__);

    printf("Checksums for verification calculated with https://crccalc.com\r\n");

    bool error = false;
    uint32_t checksum = 0;
    uint32_t expected_checksum = 0;

    // -------------------------------------------------------------------------

    // Prepare data
    uint32_t data0[] = {0x00000000UL};
    expected_checksum = 0x2144DF1CUL;

    // Calculate checksum
    checksum = crc_calculate(data0, sizeof(data0)/sizeof(uint32_t));

    // Verify the result
    error = (checksum != expected_checksum) ? true : error;

    // Print data, expected checksum and calculated checksum
    print_result(data0, sizeof(data0)/sizeof(uint32_t), expected_checksum, checksum);

    // -------------------------------------------------------------------------

    // Prepare data
    uint32_t data1[] = {0x12345678UL, 0xAA55AA55UL};
    expected_checksum = 0x13ADCCB8UL;

    // Calculate checksum
    checksum = crc_calculate(data1, sizeof(data1)/sizeof(uint32_t));

    // Verify the result
    error = (checksum != expected_checksum) ? true : error;

    // Print data, expected checksum and calculated checksum
    print_result(data1, sizeof(data1)/sizeof(uint32_t), expected_checksum, checksum);

    // -------------------------------------------------------------------------

    // Prepare data
    uint32_t data2[] =
    {
        0x00001111UL, 0x22223333UL, 0x44445555UL, 0x66667777UL,
        0x88889999UL, 0xAAAABBBBUL, 0xCCCCDDDDUL, 0xEEEEFFFFUL,
        0x00001111UL, 0x22223333UL, 0x44445555UL, 0x66667777UL,
        0x88889999UL, 0xAAAABBBBUL, 0xCCCCDDDDUL, 0xEEEEFFFFUL
    };
    expected_checksum = 0xED561F34UL;

    // Calculate checksum
    checksum = crc_calculate(data2, sizeof(data2)/sizeof(uint32_t));

    // Verify the result
    error = (checksum != expected_checksum) ? true : error;

    // Print data, expected checksum and calculated checksum
    print_result(data2, sizeof(data2)/sizeof(uint32_t), expected_checksum, checksum);

    // -------------------------------------------------------------------------

    while(1)
    {
        if(error)
        {
            led_red_on();
        }
        else
        {
            led_green_on();
        }

        // Delay
        for(volatile int i=0; i<5000; i++)
        {}

        led_green_off();
        led_red_off();

        // Delay
        for(volatile int i=0; i<5000000; i++)
        {}
    }
}

// -----------------------------------------------------------------------------
// Local function implementation
// -----------------------------------------------------------------------------

void print_result(uint32_t data[], size_t data_len, uint32_t expected_checksum,
    uint32_t calculated_checksum)
{
    printf("\r\n");
    printf("Data:\r\n");
    for(size_t i=0; i<data_len; i++)
    {
        printf("0x%08lX ", data[i]);
        if((i+1) % 4 == 0 && (i+1) < data_len)
        {
            printf("\r\n");
        }
    }
    printf("\r\n");

    printf("Expected checksum  : 0x%08lX\r\n", expected_checksum);
    printf("Calculated checksum: 0x%08lX\r\n", calculated_checksum);

    if(calculated_checksum != expected_checksum)
    {
        printf("Error: Checksum does not match expected value!\r\n");
    }
    else
    {
        printf("Success: Checksum matches expected value.\r\n");
    }
}