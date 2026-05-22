/*! ***************************************************************************
 *
 * \brief     Low level driver for the Windog Watchdog Timer (WWDT)
 * \file      wwdt.c
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
 ******************************************************************************/
#include <stdio.h>
#include "wwdt.h"

// -----------------------------------------------------------------------------
// Local type definitions
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Local function prototypes
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Local variables
// -----------------------------------------------------------------------------
volatile bool wwdt_flag = false;
extern volatile uint32_t ms;

// -----------------------------------------------------------------------------
// Local function implementation
// -----------------------------------------------------------------------------
void wwdt_init(void)
{
    // Perform the following steps to initialize WWDT:
    // 1. Enable and configure the watchdog oscillator.
    // 2. Set the watchdog timer constant reload value using TC[COUNT].
    // 3. Set the watchdog timer operating mode using MOD[WDPROTECT].
    // 4. Set a value for the watchdog window time using WINDOW[WINDOW] if the
    //    windowed operation is desired.
    // 5. Set a value for the watchdog warning interrupt using WARNINT[WARNINT]
    //    if a warning interrupt is desired.
    // 6. Enable the watchdog by writing AAh followed by 55h to FEED[FEED].

    // Enable modules and leave others unchanged
    // WWDT0: [1] = Peripheral clock is enabled
    MRCC0->MRCC_GLB_CC0_SET = MRCC_MRCC_GLB_CC0_WWDT0(1);

    // HALT: [0] = Divider clock is running
    // RESET: [0] = Divider isn't reset
    // DIV: [0000] = divider value = (DIV+1) = 1
    MRCC0->MRCC_WWDT0_CLKDIV = 0;

    // 2.
    // T_interval = T_wdclk * TC * 4
    // TC = T_interval / (T_wdclk * 4) = 1200ms / (0.001ms * 4) = 300000
    WWDT0->TC = 1200000 / 4;

    // 3.
    // WDPROTECT: [0] = Flexible (can change the value of TC[COUNT] at any time)
    WWDT0->MOD &= ~(WWDT_MOD_WDPROTECT(1));

    // 4. Not used

    // 5. Not used

    // 6. Enable the watchdog, see wwdt_enable() function

    // Enable Interrupts
    NVIC_SetPriority(WWDT0_IRQn, 0);
    NVIC_ClearPendingIRQ(WWDT0_IRQn);
    NVIC_EnableIRQ(WWDT0_IRQn);
}

void wwdt_enable(void)
{
    // WDEN: [1] = Watchdog is enabled - Timer running
    // WDTRESET: [0] = Interrupt
    WWDT0->MOD |= WWDT_MOD_WDEN(1);
    WWDT0->MOD &= ~(WWDT_MOD_WDRESET(1));

    wwdt_feed();
}

void wwdt_reset(const bool enable)
{
    if(enable)
    {
        WWDT0->MOD |= WWDT_MOD_WDRESET(1);
    }
    else
    {
        WWDT0->MOD &= ~(WWDT_MOD_WDRESET(1));
    }
}

void wwdt_feed(void)
{
    // Critical section start, see reference manual 27.6.1.4 Feed Sequence
    // (FEED)
    uint32_t m = __get_PRIMASK();
    __disable_irq();

    // Write 0xAA followed by 0x55 to FEED[FEED]
    WWDT0->FEED = WWDT_FEED_FEED(0xAA);
    WWDT0->FEED = WWDT_FEED_FEED(0x55);

    // Critical section end
    __set_PRIMASK(m);
}

void WWDT0_IRQHandler(void)
{
    // Clear pending IRQ
    NVIC_ClearPendingIRQ(WWDT0_IRQn);

    // Watchdog Timeout Flag?
    if((WWDT0->MOD & WWDT_MOD_WDTOF_MASK) != 0)
    {
        // Clear Watchdog Timeout Flag
        WWDT0->MOD &= ~(WWDT_MOD_WDTOF_MASK);

        // Handle the event
        wwdt_flag = true;

        // This print statement is for demonstration purposes only. In a real
        // application, it is recommended to avoid using printf in interrupt
        // handlers!
        printf("[% 8ld] WWDT Timeout interrupt\r\n", ms);
    }
}
