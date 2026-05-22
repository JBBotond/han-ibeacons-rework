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

#include "serial.h"

#include "eflexpwm.h"

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
static uint16_t heartbeat_duty(uint32_t time_ms);

// -----------------------------------------------------------------------------
// Local variables
// -----------------------------------------------------------------------------
static volatile uint32_t ms = 0;
static volatile uint32_t previous_ms = 0;
static const uint32_t interval_ms = 1;

// -----------------------------------------------------------------------------
// Main application
// -----------------------------------------------------------------------------
int main(void)
{
    // Generate an interrupt every 1 ms
    SysTick_Config(48000);

    serial_init(115200);
    eflexpwm_init();

    printf("eFlexPWM - Edge Aligned example\r\n");
    printf("%s build %s %s\r\n", TARGETSTR, __DATE__, __TIME__);

    // Globally enable interrupts
    __enable_irq();

    while(1)
    {
        uint32_t current_ms = ms;

        // --------------------------------------------------------------------
        if((current_ms - previous_ms) >= interval_ms)
        {
            previous_ms = current_ms;

            uint16_t duty_cycle = heartbeat_duty(current_ms);

            // eflexpwm_set_red(duty_cycle);
            eflexpwm_set_green(duty_cycle);
            // eflexpwm_set_blue(duty_cycle);
        }
    }
}

// -----------------------------------------------------------------------------
// Local function implementation
// -----------------------------------------------------------------------------

void SysTick_Handler(void)
{
    ms++;
}

/*!
 * \brief Calculate the duty cycle for a heartbeat effect
 *
 * This function generates a heartbeat effect by smoothly fading the LED in and
 * out over a period of 2 seconds, followed by a 1 second pause. The duty cycle
 * is calculated using an ease-in/ease-out curve for a more natural effect.
 *
 * The most accurate effect is achieved when this function is called every 1 ms,
 * as it relies on the time in milliseconds to calculate the phase of the
 * heartbeat cycle.
 *
 * \param[in]  time_ms  The current time in milliseconds
 *
 * \return The calculated 16-bit duty cycle (0-65535)
 */
static uint16_t heartbeat_duty(uint32_t time_ms)
{
    uint32_t phase = time_ms % 3000U;

    // Pause period (2000-3000 ms)?
    if(phase >= 2000U)
    {
        return 0;
    }

    // Active fade period (0-2000 ms)
    uint32_t x;
    uint32_t half = 1000U;

    if(phase < half)
    {
        // Fade up
        x = phase;
    }
    else
    {
        // Fade down
        x = 2000U - phase;
    }

    // Apply ease-in/out curve: y = x^2
    uint32_t y = (x * x) / half;
    uint32_t duty = (y * 0xFFFFU) / half;

    return (uint16_t)duty;
}
