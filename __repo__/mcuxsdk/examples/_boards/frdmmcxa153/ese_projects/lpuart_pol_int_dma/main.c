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
#include <MCXA153.h>
#include <stdio.h>
#include <string.h>
#include "benchmark.h"
#include "gpio_output.h"
#include "lpuart0_polling.h"
#include "lpuart0_interrupt.h"
#include "lpuart0_dma.h"

// -----------------------------------------------------------------------------
// Local type definitions
// -----------------------------------------------------------------------------
#ifdef DEBUG
#define TARGETSTR "Debug"
#else
#define TARGETSTR "Release"
#endif

#define N_TESTDATA (16*1024)

// -----------------------------------------------------------------------------
// Local function prototypes
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Local variables
// -----------------------------------------------------------------------------
static uint8_t testdata[N_TESTDATA];

// -----------------------------------------------------------------------------
// Main application
// -----------------------------------------------------------------------------
int main(void)
{
    gpio_output_init();
    benchmark_init();
    lpuart0_pol_init(115200);

    printf("\r\n\r\n");
    printf("Polling vs Interrupts vs DMA - %s\r\n", TARGETSTR);
    printf("Build %s %s\r\n", __DATE__, __TIME__);
    printf("\r\n");
    printf("--------------------------------------------------------\r\n");
    printf("\r\n");
    printf("This demo compares CPU activity between software\r\n" \
           "polling, interrupt handling, and direct memory\r\n" \
           "access (DMA) during UART data transmission.\r\n");
    printf("\r\n");
    printf("Transmitting 16kB of data via UART at 115200-8n1 will take:\r\n" \
           "(1+8+1) * 16384 * 1/115200 = 1.42 seconds\r\n");
    printf("\r\n");
    printf("--------------------------------------------------------\r\n");

    // Prepare data for testing
    for(uint32_t i=0; i<N_TESTDATA; ++i)
    {
        testdata[i] = '0';
    }

    // -------------------------------------------------------------------------
    // Start a measurement: Polling
    // -------------------------------------------------------------------------
    printf("\r\n");
    printf("Press any key to start a polled transfer of 16kB\r\n");
    printf("How long do you think the CPU is busy?\r\n");
    getchar();

    // Reset LPUART0
    MRCC0->MRCC_GLB_CC0_CLR = MRCC_MRCC_GLB_CC0_LPUART0(1);

    // Start a measurement
    lpuart0_pol_init(115200);
    lpuart0_pol_tx(testdata, N_TESTDATA);

    // Wait unit finished
    while(pol_tx_is_done_flag == false)
    {}

    // Print the result
    printf("\r\nPolling transfer:\r\n");
    printf("Total transfer time: %9.1f microseconds\r\n", (double)pol_total_us);
    printf("CPU active time    : %9.1f microseconds\r\n", (double)pol_active_us);

    // -------------------------------------------------------------------------
    // Start a measurement: Interrupt
    // -------------------------------------------------------------------------
    printf("\r\n");
    printf("Press any key to start a interrupt transfer of 16kB\r\n");
    printf("How long do you think the CPU is busy?\r\n");
    getchar();

    // Reset LPUART0
    MRCC0->MRCC_GLB_CC0_CLR = MRCC_MRCC_GLB_CC0_LPUART0(1);

    lpuart0_int_init(115200);
    lpuart0_int_tx(testdata, N_TESTDATA);

    // Wait unit finished
    while(int_tx_is_done_flag == false)
    {}

    // Print the result
    printf("\r\nInterrupt transfer:\r\n");
    printf("Total transfer time: %9.1f microseconds\r\n", (double)int_total_us);
    printf("CPU active time    : %9.1f microseconds\r\n", (double)int_active_us);

    // -------------------------------------------------------------------------

    printf("\r\n");
    printf("Press any key to start a DMA transfer of 16kB\r\n");
    printf("How long do you think the CPU is busy?\r\n");
    getchar();

    // Reset LPUART0
    MRCC0->MRCC_GLB_CC0_CLR = MRCC_MRCC_GLB_CC0_LPUART0(1);

    lpuart0_dma_init(115200);
    lpuart0_dma_tx(testdata, N_TESTDATA);

    // Wait unit finished
    while(dma_tx_is_done_flag == false)
    {}

    // Print the result
    printf("\r\nDMA transfer:\r\n");
    printf("Total transfer time: %9.1f microseconds\r\n", (double)dma_total_us);
    printf("CPU active time    : %9.1f microseconds\r\n", (double)dma_active_us);

    // -------------------------------------------------------------------------
    // Print summary
    // -------------------------------------------------------------------------
    printf("\r\n\r\n");
    printf("Summary       Total     |    CPU\r\n");
    printf("            transfer    |   active\r\n");
    printf("           -------------+-------------\r\n");
    printf("Polling  : %9.1f us | %9.1f us\r\n", (double)pol_total_us, (double)pol_active_us);
    printf("Interrupt: %9.1f us | %9.1f us\r\n", (double)int_total_us, (double)int_active_us);
    printf("DMA      : %9.1f us | %9.1f us\r\n", (double)dma_total_us, (double)dma_active_us);

    #ifdef DEBUG
    printf("\r\n\r\n");
    printf("Copy-and-paste this summary, switch to the\r\n" \
           "Release build, and rerun this test. What\r\n" \
           "do you think the results will be?\r\n");
    #endif

    while(1)
    {}
}

// -----------------------------------------------------------------------------
// Local function implementation
// -----------------------------------------------------------------------------
