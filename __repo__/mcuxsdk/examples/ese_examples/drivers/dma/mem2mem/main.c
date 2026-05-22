/*! ***************************************************************************
 *
 * \brief     Main application
 * \file      main.c
 * \author    Hugo Arends
 * \date      February 2026
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
#include <stdio.h>
#include <string.h>

#include "leds.h"
#include "serial.h"

#include "dma.h"

// -----------------------------------------------------------------------------
// Local type definitions
// -----------------------------------------------------------------------------
#ifdef DEBUG
#define TARGETSTR "Debug"
#else
#define TARGETSTR "Release"
#endif

#define N_BYTES (1024)
// -----------------------------------------------------------------------------
// Local function prototypes
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Local variables
// -----------------------------------------------------------------------------
static uint8_t src[N_BYTES] = {0};
static uint8_t dst[N_BYTES] = {0};

static volatile uint32_t ms = 0;
static const volatile uint32_t led_on_ms = 100;
static const volatile uint32_t led_off_ms = 1900;

static void (*led_on)(void) = led_green_on;
static void (*led_off)(void) = led_green_off;

// -----------------------------------------------------------------------------
// Main application
// -----------------------------------------------------------------------------
int main(void)
{
    // Generate an interrupt every 1 ms
    SysTick_Config(48000);

    serial_init(115200);
    leds_init();

    dma_mem2mem_init();

    printf("DMA - MEM2MEM example\r\n");
    printf("%s build %s %s\r\n", TARGETSTR, __DATE__, __TIME__);

    printf("Initialize data\r\n");

    // Initialize data
    for(uint32_t i=0; i<N_BYTES; i++)
    {
        src[i] = (uint8_t)i;
    }

    printf("DMA mem2mem - start\r\n");

    // Copy data
    dma_mem2mem_start(src, dst, N_BYTES);

    // Wait until data copied
    while(!dma_mem2mem_done())
    {}

    printf("DMA mem2mem - done\r\n");

    // Check the results
    if(memcmp(src, dst, N_BYTES) == 0)
    {
        printf("DMA mem2mem - success\r\n");
        led_on = led_green_on;
        led_off = led_green_off;
    }
    else
    {
        printf("DMA mem2mem - failed\r\n");
        led_on = led_red_on;
        led_off = led_red_off;
    }

    printf("Press reset to restart\r\n");

    // Blink LED
    while(1)
    {
        ms = 0;

        led_on();

        while(ms < led_on_ms)
        {}

        led_off();

        while(ms < led_off_ms)
        {}
    }
}

// -----------------------------------------------------------------------------
// Local function implementation
// -----------------------------------------------------------------------------
void SysTick_Handler(void)
{
    ms++;
}
