/*! ***************************************************************************
 *
 * \brief     Main application
 * \file      main.c
 * \author    Hugo Arends
 * \date      March 2026
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
#include <stdio.h>
#include <string.h>

#include "leds.h"
#include "switches.h"
#include "serial.h"

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
void SysTick_Handler_Relocated(void);

// -----------------------------------------------------------------------------
// Local variables
// -----------------------------------------------------------------------------
static volatile uint32_t ms = 0;

// The new interrupt vector table in RAM must be aligned. The maximum number of
// interrupt vectors is 128 (although only 96 are implemented in the MCXA153).
// Each vector stores a 32-bit address, so the alignment is equal to
// 128 * 4 = 512 bytes.
static uint32_t interrupt_vectors_ram[96] __attribute__ ((aligned(512))) = {0};

// -----------------------------------------------------------------------------
// Main application
// -----------------------------------------------------------------------------
int main(void)
{
    leds_init();
    sw_init();
    serial_init(115200);

    SysTick_Config(48000);

    printf("Interrupt vector table relocation example\r\n");
    printf("%s build %s %s\r\n", TARGETSTR, __DATE__, __TIME__);

    // Copy original vector table from FLASH (starting at address 0x00000000)
    // to the new location in RAM (interrupt_vectors_ram array).
    memcpy((void *)interrupt_vectors_ram, (void *)0x00000000, sizeof(interrupt_vectors_ram));

    // Update specific handlers for this example
    interrupt_vectors_ram[16 + SysTick_IRQn] = (uint32_t)SysTick_Handler_Relocated;
    interrupt_vectors_ram[16 + GPIO1_IRQn] = (uint32_t)GPIO1_IRQHandler_Relocated;

    // Force the previous instructions to complete before updating the VTOR
    // register.
    __DSB();

    while(1)
    {
        // --------------------------------------------------------------------
        if(serial_rxcnt() > 0)
        {
            // Echo the received character
            int c = getchar();
            putchar(c);
        }

        // --------------------------------------------------------------------
        if(sw2_pressed())
        {
            led_green_off();
            led_red_off();

            __disable_irq();

            // Check the current vector table location and switch to the
            // other one.
            if(SCB->VTOR == 0x00000000UL)
            {
                SCB->VTOR = (uint32_t)interrupt_vectors_ram;

                // Ensure the changes are seen by subsequent instructions
                __DSB();
                __ISB();

                // Interrupts or exceptions that are taken after this
                // point are guaranteed to use the updated vector table.
                __enable_irq();

                printf("SW2: Relocating vector table to RAM: 0x%08X\r\n",
                    (unsigned int)(SCB->VTOR));
            }
            else
            {
                SCB->VTOR = 0x00000000UL;

                // Ensure the changes are seen by subsequent instructions
                __DSB();
                __ISB();

                // Interrupts or exceptions that are taken after this
                // point are guaranteed to use the updated vector table.
                __enable_irq();

                printf("SW2: Relocating vector table to FLASH: 0x%08X\r\n",
                    (unsigned int)(SCB->VTOR));
            }
        }

        // --------------------------------------------------------------------
        if(sw3_pressed())
        {
            printf("SW3: Triggered by function from vector table in FLASH\r\n");
        }

        // --------------------------------------------------------------------
        if(sw3_relocated_pressed())
        {
            printf("SW3: Triggered by function from vector table in RAM\r\n");
        }
    }
}

// -----------------------------------------------------------------------------
// Local function implementation
// -----------------------------------------------------------------------------
void SysTick_Handler(void)
{
    ms++;

    ms = ms % 1000;

    if(ms == 0)
    {
        led_green_toggle();
    }
}

void SysTick_Handler_Relocated(void)
{
    ms++;

    ms = ms % 1000;

    if(ms == 0)
    {
        led_red_toggle();
    }
}
