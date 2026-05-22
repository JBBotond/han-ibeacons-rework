/*! ***************************************************************************
 *
 * \brief     Main application
 * \file      main.c
 * \author    Hugo Arends
 * \date      February 2026
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

#include "buzzer.h"
#include "leds.h"
#include "oled.h"
#include "serial.h"

#include "shell.h"
#include "fatfs_shell.h"
#include "led_shell.h"

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
volatile uint32_t ms = 0;

// -----------------------------------------------------------------------------
// Main application
// -----------------------------------------------------------------------------
int main(void)
{
    // Generate an interrupt every 1 ms
    SysTick_Config(48000);

    leds_init();
    serial_init(115200);
    oled_init();

    printf("SDCARD and FatFS - Shell\r\n");
    printf("%s build %s %s\r\n", TARGETSTR, __DATE__, __TIME__);

    oled_setorientation(1);
    oled_drawbitmap(shell);
    oled_update();

    while(ms < 1000)
    {}

    shell_command_t commands[32];
    char cwd_buffer[256];
    shell_context_t shell_ctx;

    shell_init(&shell_ctx, commands, 0, cwd_buffer, sizeof(cwd_buffer));

    // Initialize all shell modules
    fatfs_shell_init(&shell_ctx);
    led_shell_init(&shell_ctx);

    printf("Starting shell\n");
    shell_run(&shell_ctx);

    printf("Shell application ended\n");
    printf("Press reset (SW1) to restart\n");

    while(1)
    {

    }
}

// -----------------------------------------------------------------------------
// Local function implementation
// -----------------------------------------------------------------------------
void SysTick_Handler(void)
{
    ms++;
}
