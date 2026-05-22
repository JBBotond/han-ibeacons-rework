/*! ***************************************************************************
 *
 * \brief     Main application
 * \file      main.c
 * \author    Hugo Arends
 * \date      January 2026
 *
 *            FRDM-MCXA153              Micro SDCARD Adapter Module
 *            ------------+            +---------------------------+
 *                        |            |                           |
 *                    3V3 +------------+ 3V3                       |
 *                    GND +------------+ GND                       |
 *                        |            |                           |
 *              P1_0/SDO  +------------+ MOSI                      |
 *              P1_2/SDI  +------------+ MISO                      |
 *              P1_1/SCK  +------------+ SCK                       |
 *              P1_3/GPIO +------------+ CS                        |
 *                        |            |                           |
 *            ------------+            +---------------------------+
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
 *****************************************************************************/
#include <board.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>

#include "leds.h"
#include "serial.h"

#include "ff.h"

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

// -----------------------------------------------------------------------------
// Local variables
// -----------------------------------------------------------------------------
// Do not declare static, because it is reused in diskio.c file for creating
// delays
volatile uint32_t ms = 0;

// FatFs work area needed for each volume
FATFS FatFs;

// File object needed for each open file
FIL Fil;

// -----------------------------------------------------------------------------
// Main application
// -----------------------------------------------------------------------------
int main(void)
{
    leds_init();
    serial_init(115200);

    // Generate an interrupt every 1 ms
    SysTick_Config(48000);

    printf("FatFS - LPSPI example\r\n");
    printf("%s build %s %s\r\n", TARGETSTR, __DATE__, __TIME__);

    printf("Connect a microSD Card Adapter Module to the FRDM-MCXA153 as follows:\r\n");
    printf("\r\n");
    printf("FRDM-MCXA153       Micro SDCARD Adapter Module\r\n");
    printf("------------+     +---------------------------+\r\n");
    printf("        3V3 +-----+ 3V3                       |\r\n");
    printf("        GND +-----+ GND                       |\r\n");
    printf("   P1_0/SDO +-----+ MOSI                      |\r\n");
    printf("   P1_1/SCK +-----+ SCK                       |\r\n");
    printf("   P1_2/SDI +-----+ MISO                      |\r\n");
    printf("   P1_3/CS  +-----+ CS                        |\r\n");
    printf("------------+     +---------------------------+\r\n");
    printf("\r\n");

    FRESULT fr;

    // Give a work area to the default drive
    f_mount(&FatFs, "", 0);

    char *filename = "message.txt";
    printf("[%08lu] Filename: %s\r\n", ms, filename);

    // Create a file
    fr = f_open(&Fil, filename, FA_WRITE | FA_CREATE_ALWAYS);

    if(fr == FR_OK)
    {
        printf("[%08lu] Writing data to file\r\n", ms);

        f_puts("Hello World!\n", &Fil);
        f_puts("\n", &Fil);
        f_puts("This file is written by the FRDM-MCXA153\n", &Fil);
        f_puts("\n", &Fil);
        f_puts("FRDM-MCXA153 and FatFS LPSPI example\n", &Fil);
        f_puts("\n", &Fil);
        f_puts("made by ESE\n", &Fil);
        f_puts("HAN Embedded Systems Engineering\n", &Fil);

        printf("[%08lu] Closing file\r\n", ms);

        // Close the file
        fr = f_close(&Fil);

        if(fr == FR_OK)
        {
            // Blinks green LED if data written well
            led_green_on();

            printf("[%08lu] File created and written successfully\r\n", ms);

            uint32_t timeout_ms = ms + 50;

            while(ms < timeout_ms)
            {}

            led_green_off();
        }
        else
        {
            printf("[%08lu] ERROR File not written\r\n", ms);
        }
    }
    else
    {
        printf("[%08lu] ERROR File not created for writing\r\n", ms);
    }

    char line[100];

    printf("[%08lu] Opening file\r\n", ms);

    // Open a text file
    fr = f_open(&Fil, filename, FA_READ);

    if(fr == FR_OK)
    {
        // Blinks blue LED while data read
        led_blue_on();

        printf("[%08lu] Reading file\r\n", ms);
        printf("[%08lu] ------------------------------------------\r\n", ms);

        // Read every line and print it
        while(f_gets(line, sizeof line, &Fil))
        {
            printf("[%08lu] %s", ms, line);
        }

        printf("[%08lu] ------------------------------------------\r\n", ms);
        printf("[%08lu] Closing file\r\n", ms);

        // Close the file
        f_close(&Fil);

        led_blue_off();
    }
    else
    {
        printf("[%08lu] ERROR File not opened for reading\r\n", ms);
    }

    printf("[%08lu] Unmounting filesystem\r\n", ms);

    f_unmount("");

    // Done
    while(1)
    {}
}

// -----------------------------------------------------------------------------
// Local function implementation
// -----------------------------------------------------------------------------
void SysTick_Handler(void)
{
    ms++;
}
