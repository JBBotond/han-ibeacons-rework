/*! ***************************************************************************
 *
 * \brief     Low level driver for the Standard Counter or Timer (CTIMER)
 * \file      ctimer.h
 * \author    Hugo Arends
 * \date      January 2025
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
#include "ctimer.h"
#include "serial.h"

// -----------------------------------------------------------------------------
// Local type definitions
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Local function prototypes
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Local variables
// -----------------------------------------------------------------------------
SemaphoreHandle_t xCountingSemaphore;

// -----------------------------------------------------------------------------
// Local function implementation
// -----------------------------------------------------------------------------
void ctimer_init(void)
{
    // From section 26.1.5 Initialization (NXP, 2024)
    //
    // 1. Select a clock source for the CTIMER using MRCC_CTIMER0_CLKSEL,
    //    MRCC_CTIMER1_CLKSEL, and MRCC_CTIMER2_CLKSEL registers.
    // 2. Enable the clock to the CTIMER via the
    //    CTIMERGLOBALSTARTEN[CTIMER0_CLK_EN],
    //    CTIMERGLOBALSTARTEN[CTIMER1_CLK_EN], and
    //    CTIMERGLOBALSTARTEN[CTIMER2_CLK_EN] fields. This enables the register
    //    interface and the peripheral function clock.
    // 3. Clear the CTIMER peripheral reset using the MRCC_GLB_RST0 registers.
    // 4. Each CTIMER provides interrupts to the NVIC. See MCR and CCR registers
    //    in the CTIMER register section for match and capture events. For
    //    interrupt connections, see the attached NVIC spreadsheet.
    // 5. Select timer pins and pin modes as needed through the relevant PORT
    //    registers.
    // 6. The CTIMER DMA request lines are connected to the DMA trigger inputs
    //    via the DMAC0_ITRIG_INMUX registers (See Memory map and register
    //    definition). Note that timer DMA request outputs are connected to DMA
    //    trigger inputs.

    // 1.
    //
    // MUX: [101] = CLK_1M
    MRCC0->MRCC_CTIMER1_CLKSEL = MRCC_MRCC_CTIMER1_CLKSEL_MUX(0b101);

    // HALT: [0] = Divider clock is running
    // RESET: [0] = Divider isn't reset
    // DIV: [0000] = divider value = (DIV+1) = 1
    MRCC0->MRCC_CTIMER1_CLKDIV = 0;

    // 2.
    //
    // CTIMER1_CLK_EN: [1] = CTIMER 1 function clock enabled
    SYSCON->CTIMERGLOBALSTARTEN |= SYSCON_CTIMERGLOBALSTARTEN_CTIMER1_CLK_EN(1);

    // 3.
    //
    // Enable modules and leave others unchanged
    // CTIMER1: [1] = Peripheral clock is enabled
    MRCC0->MRCC_GLB_CC0_SET = MRCC_MRCC_GLB_CC0_CTIMER1(1);

    // Release modules from reset and leave others unchanged
    // CTIMER1: [1] = Peripheral is released from reset
    MRCC0->MRCC_GLB_RST0_SET = MRCC_MRCC_GLB_RST0_CTIMER1(1);

    // 4.
    //
    // Specifies the prescale value. 1 MHz / 1 = 1 MHz
    CTIMER1->PR = 0;

    // Match value for match register 0. 1 MHz / 1000 = 1 kHz
    CTIMER1->MR[0] = 1000-1;

    // MR0S: [0] = Does not stop Timer Counter (TC) if MR0 matches Timer Counter
    //             (TC)
    // MR0R: [1] = Resets Timer Counter (TC) if MR0 matches its value.
    // MR0I: [1] = Generates an interrupt when MR0 matches the value in Timer
    //             Counter (TC).
    CTIMER1->MCR |= CTIMER_MCR_MR0R(1) | CTIMER_MCR_MR0I(1);

    // 5.
    //
    // Not used.

    // 6.
    //
    // Not used.

    // Enable Interrupts
    NVIC_SetPriority(CTIMER1_IRQn, 7);
    NVIC_ClearPendingIRQ(CTIMER1_IRQn);
    NVIC_EnableIRQ(CTIMER1_IRQn);

    // CEN: [1] = Enables the counters.
    CTIMER1->TCR |= CTIMER_TCR_CEN(1);
}

void CTIMER1_IRQHandler(void)
{
    static uint32_t cnt = 0;

    // Clear pending IRQ
    NVIC_ClearPendingIRQ(CTIMER1_IRQn);

    /* The xHigherPriorityTaskWoken parameter must be initialized to pdFALSE as
    it will get set to pdTRUE inside the interrupt safe API function if a
    context switch is required. */
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    // Interrupt generated by MR0?
    if((CTIMER1->IR & CTIMER_IR_MR0INT(1)) != 0)
    {
        // Clear status flag by writing 1
        CTIMER1->IR |= CTIMER_IR_MR0INT(1);

        // Handle the event
        if(++cnt == 2000)
        {
            cnt = 0;

            // In a real application you DO NOT print information in an ISR.
            // This is here now for demonstrating that an interrupt has been generated!
            vSerialPutString( "[ISR handler ] Interrupt generated\r\n" );

            /* 'Give' the semaphore multiple times. The first will unblock the deferred
            interrupt handling task, the following 'gives' are to demonstrate that the
            semaphore latches the events to allow the task to which interrupts are deferred
            to process them in turn, without events getting lost. This simulates multiple
            interrupts being received by the processor, even though in this case the events
            are simulated within a single interrupt occurrence. */
            xSemaphoreGiveFromISR( xCountingSemaphore, &xHigherPriorityTaskWoken );
            xSemaphoreGiveFromISR( xCountingSemaphore, &xHigherPriorityTaskWoken );
            xSemaphoreGiveFromISR( xCountingSemaphore, &xHigherPriorityTaskWoken );

            /* Pass the xHigherPriorityTaskWoken value into portYIELD_FROM_ISR(). If
            xHigherPriorityTaskWoken was set to pdTRUE inside xSemaphoreGiveFromISR()
            then calling portYIELD_FROM_ISR() will request a context switch. If
            xHigherPriorityTaskWoken is still pdFALSE then calling
            portYIELD_FROM_ISR() will have no effect. */
            portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
        }
    }
}
