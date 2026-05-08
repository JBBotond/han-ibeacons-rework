/*! ***************************************************************************
 *
 * \brief     Main application
 * \file      main.c
 * \author    Hugo Arends
 * \date      September 2024
 *
 *            Demonstration project that forwards data from LPUART2 to serial
 *            (LPUART0). The data is not interpreted, it is forwarded in both
 *            directions.
 *
 *             MCU-Link                FRDM-MCXA153                  Device
 *            +--------+    +----------------------------------+    +--------
 *            |      RX|<---|LPUART0_TX/P0_3 < LPUART2_RX/P1_4 |<---| TXD
 *        <-->|USB   TX|--->|LPUART0_RX/P0_2 > LPUART2_TX/P1_5 |--->| RXD
 *            |        |    |                              3V3 |----| VCC
 *            |        |    |                              GND |----| GND
 *            +--------+    +----------------------------------+    +--------
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
#include <MCXA153.h>
#include <stdio.h>
#include <string.h>

#include "serial.h"
#include "lpuart2.h"

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
void atSendCommand(char *command);
// -----------------------------------------------------------------------------
// Local variables
// -----------------------------------------------------------------------------
const char hardware_connection[] =
    "Hardware connection:\r\n"
    "\r\n"
    "     MCU-Link                FRDM-MCXA153                  Device\r\n"
    "    +--------+    +----------------------------------+    +--------\r\n"
    "    |      RX|<---|LPUART0_TX/P0_3 < LPUART2_RX/P1_4 |<---| TXD\r\n"
    "<-->|USB   TX|--->|LPUART0_RX/P0_2 > LPUART2_TX/P1_5 |--->| RXD\r\n"
    "    |        |    |                              3V3 |----| VCC\r\n"
    "    |        |    |                              GND |----| GND\r\n"
    "    +--------+    +----------------------------------+    +--------\r\n";

char *atCommand = "AT+DISI?\r\n";

// -----------------------------------------------------------------------------
// Main application
// -----------------------------------------------------------------------------
int main(void)
{
    serial_init(115200);
    lpuart2_init(9600);

    printf("Serial data forwarder\r\n");
    printf("%s build %s %s\r\n", TARGETSTR, __DATE__, __TIME__);

    printf("\r\n");
    printf(hardware_connection);
    printf("\r\n");
    
    atSendCommand(atCommand);

    while(1)
    {
        // Data available from serial?
        if(serial_rxcnt() > 0)
        {
            // Get the data from serial (LPUART0)
            uint8_t data = serial_getchar();

            // Forward the data to LPUART2
            lpuart2_putchar(data);
        }

        // Data available from module?
        if(lpuart2_rxcnt() > 0)
        {
            // Get the data from LPUART2
            uint8_t data = lpuart2_getchar();

            // Forward the data to serial (LPUART0)
            serial_putchar(data);
        }
    }
}

// -----------------------------------------------------------------------------
// Local function implementation
// -----------------------------------------------------------------------------
void atSendCommand(char *command) {
    while(*command)
            lpuart2_putchar(*command++);
}