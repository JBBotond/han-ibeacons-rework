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

#include "lpspi_master.h"

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
void test1(void);
void test2(void);

// -----------------------------------------------------------------------------
// Local variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Main application
// -----------------------------------------------------------------------------
int main(void)
{
    serial_init(115200);
    lpspi_master_init();

    printf("LPSPI master polling example\r\n");
    printf("%s build %s %s\r\n", TARGETSTR, __DATE__, __TIME__);

    while(1)
    {
        test1();

        // Delay
        for(volatile uint32_t i = 0; i < 10000000; i++)
        {}

        test2();

        // Delay
        for(volatile uint32_t i = 0; i < 10000000; i++)
        {}
    }
}

// -----------------------------------------------------------------------------
// Local function implementation
// -----------------------------------------------------------------------------
void test1(void)
{
    uint8_t tx_data[] = "Hello World!";
    uint8_t rx_data[sizeof(tx_data)] = {0};

    printf("Test 1: ");

    lpspi_transceive(tx_data, rx_data, sizeof(tx_data));

    if(memcmp(tx_data, rx_data, sizeof(tx_data)) == 0)
    {
        printf("MATCH\r\n");
    }
    else
    {
        printf("MISMATCH\r\n");
    }

    printf("  tx_data: ");
    for(uint32_t i = 0; i < sizeof(tx_data); i++)
    {
        printf("%02X ", tx_data[i]);
    }
    printf("\r\n");

    printf("  rx_data: ");
    for(uint32_t i = 0; i < sizeof(rx_data); i++)
    {
        printf("%02X ", rx_data[i]);
    }
    printf("\r\n");
}

void test2(void)
{
    uint8_t rx_data[16] = {0};

    uint8_t dummy = 0xFF;

    printf("Test 2: ");

    lpspi_set_dummy(dummy);
    lpspi_receive(rx_data, sizeof(rx_data));

    bool match = true;

    for(uint32_t i = 0; i < sizeof(rx_data); i++)
    {
        if(rx_data[i] != 0xFF)
        {
            match = false;
            break;
        }
    }

    if(match == true)
    {
        printf("MATCH\r\n");
    }
    else
    {
        printf("MISMATCH\r\n");
    }

    printf("  tx_data: ");
    for(uint32_t i = 0; i < sizeof(rx_data); i++)
    {
        printf("%02X ", dummy);
    }
    printf("\r\n");

    printf("  rx_data: ");
    for(uint32_t i = 0; i < sizeof(rx_data); i++)
    {
        printf("%02X ", rx_data[i]);
    }
    printf("\r\n");
}
