/*! ***************************************************************************
 *
 * \brief     Low level driver for a buzzer using the Standard Counter or Timer
 *            (CTIMER)
 * \file      buzzer.c
 * \author    Hugo Arends
 * \date      January 2026
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
#include "buzzer.h"

// -----------------------------------------------------------------------------
// Local type definitions
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Local function prototypes
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Local variables
// -----------------------------------------------------------------------------
extern volatile uint32_t ms;

// -----------------------------------------------------------------------------
// Local function implementation
// -----------------------------------------------------------------------------

/*!
 * \brief Initializes the buzzer driver
 *
 * Resources:
 * - P3_30 | CT0_MAT2
 *
 * Configures CTIMER0 to generate a 50% PWM signal on MAT2 with a frequency
 * range of 0 Hz to 20 kHz.
 */
void buzzer_init(void)
{
    // 1.
    //
    // MUX: [101] = CLK_1M
    MRCC0->MRCC_CTIMER0_CLKSEL = MRCC_MRCC_CTIMER0_CLKSEL_MUX(0b101);

    // HALT: [0] = Divider clock is running
    // RESET: [0] = Divider isn't reset
    // DIV: [0000] = divider value = (DIV+1) = 1
    MRCC0->MRCC_CTIMER0_CLKDIV = 0;

    // 2.
    //
    // CTIMER0_CLK_EN: [1] = CTIMER 0 function clock enabled
    SYSCON->CTIMERGLOBALSTARTEN |= SYSCON_CTIMERGLOBALSTARTEN_CTIMER0_CLK_EN(1);

    // 3.
    //
    // Enable modules and leave others unchanged
    // CTIMER0: [1] = Peripheral clock is enabled
    // PORT3: [1] = Peripheral clock is enabled
    MRCC0->MRCC_GLB_CC1_SET = MRCC_MRCC_GLB_CC1_PORT3(1);
    MRCC0->MRCC_GLB_CC0_SET = MRCC_MRCC_GLB_CC0_CTIMER0(1);

    // Release modules from reset and leave others unchanged
    // CTIMER0: [1] = Peripheral is released from reset
    // PORT3: [1] = Peripheral is released from reset
    MRCC0->MRCC_GLB_RST1_SET = MRCC_MRCC_GLB_CC1_PORT3(1);
    MRCC0->MRCC_GLB_RST0_SET = MRCC_MRCC_GLB_RST0_CTIMER0(1);

    // 1. & 2.
    //
    // Configure P3_30
    // LK : [1] = Locks this PCR
    // INV: [0] = Does not invert
    // IBE: [0] = Disables
    // MUX: [0100] = Alternative 4 (CT0_MAT2)
    // DSE: [0] = low drive strength is configured on the corresponding pin,
    //            if the pin is configured as a digital output
    // ODE: [0] = Disables
    // SRE: [0] = Fast
    // PE:  [0] = Disables
    // PS:  [0] = n.a.
    PORT3->PCR[30] = PORT_PCR_LK(1) | PORT_PCR_MUX(4);

    // 4.
    //
    // Specifies the prescale value. 1 MHz / 1 = 1 MHz
    CTIMER0->PR = 0;

    // CT0_MAT3: TOP
    // CT0_MAT2: TOP / 2 (50% duty cycle)
    CTIMER0->MR[3] = 500000UL;
    CTIMER0->MR[2] = 250000UL;

    // MR2S: [0] = Does not stop Timer Counter (TC) if MR2 matches Timer Counter
    //             (TC)
    // MR2R: [0] = Does not reset Timer Counter (TC) if MR2 matches its value.
    // MR2I: [0] = Does not generate an interrupt when MR2 matches the value in Timer
    //             Counter (TC).
    // MR3S: [0] = Does not stop Timer Counter (TC) if MR3 matches Timer Counter
    //             (TC)
    // MR3R: [1] = Resets Timer Counter (TC) if MR3 matches its value.
    // MR3I: [0] = Does not generate an interrupt when MR3 matches the value in Timer
    //             Counter (TC).
    CTIMER0->MCR = CTIMER_MCR_MR3R(1);

    // Configure match outputs as PWM outputs.
    CTIMER0->PWMC |= CTIMER_PWMC_PWMEN3(1) | CTIMER_PWMC_PWMEN2(1);

    // CEN: [1] = Enables the counters.
    CTIMER0->TCR |= CTIMER_TCR_CEN(1);
}

/*!
 * \brief Sets the buzzer frequency
 *
 * \param frequency_hz Frequency in Hz. Set to 0 to turn off the buzzer.
 *                     Maximum frequency is 20 kHz.
 */
void buzzer_set(uint32_t frequency_hz)
{
    // Turn off buzzer if frequency is 0 Hz
    if(frequency_hz == 0)
    {
        CTIMER0->MR[3] = 0UL;
        CTIMER0->MR[2] = 0UL;
        return;
    }

    // Limit maximum frequency to 20 kHz
    if(frequency_hz > 20000)
    {
        frequency_hz = 20000;
    }

    // Calculate TOP value
    uint32_t top = 1000000UL / frequency_hz;

    // Set TOP value in match register MAT3
    CTIMER0->MR[3] = top;

    // Set 50% duty cycle match value in register MAT2
    CTIMER0->MR[2] = top >> 1;

    // Restart the counter
    CTIMER0->TC = 0;
}

/*!
 * \brief Plays a melody in a blocking way. This means that the function
 *        does not return until the melody has finished playing.
 *
 * \param melody Pointer to the melody to play
 */
void buzzer_playmelody_blocking(const melody_t *melody)
{
    for(uint32_t i = 0; i < melody->length; i++)
    {
        int note = melody->notes[i];
        int duration = melody->durations[i];

        buzzer_set(note);

        // Calculate duration in milliseconds
        uint32_t note_duration_ms = 1000UL / duration;

        // Wait for the duration of the note
        uint32_t current_ms = ms;
        while(ms < current_ms + note_duration_ms)
        {}

        // Brief pause between notes (25% of the note duration)
        buzzer_set(0);
        current_ms = ms;
        while(ms < current_ms + (note_duration_ms * 0.25f))
        {}
    }
}
