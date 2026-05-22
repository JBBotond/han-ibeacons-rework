#include <board.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>

#include "leds.h"
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

// -----------------------------------------------------------------------------
// Local variables
// -----------------------------------------------------------------------------
static volatile uint32_t ms = 0;
static volatile uint32_t previous_ms = 0;
static const uint32_t interval_ms = 500;

// -----------------------------------------------------------------------------
// Main application
// -----------------------------------------------------------------------------
int main(void)
{
    leds_init();
    serial_init(115200);

    // Generate an interrupt every 1 ms
    SysTick_Config(48000);

    printf("Getting started\r\n");
    printf("%s build %s %s\r\n", TARGETSTR, __DATE__, __TIME__);

    // Globally enable interrupts
    __enable_irq();

    while(1)
    {
        // Wait for interrupt
        __WFI();

        uint32_t current_ms = ms;

        // Interval milliseconds passed?
        if((current_ms - previous_ms) >= interval_ms)
        {
            previous_ms = current_ms;

            // Toggle the LED
            led_green_toggle();
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
