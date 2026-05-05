/*! ***************************************************************************
 * \brief  KidBytes View — Step 1 test
 * \file   main.c
 *
 * Test: screen clears to black, prints "display_drv OK" at (0,0).
 *****************************************************************************/
#include <board.h>
#include "serial.h"
#include "display_drv.h"

volatile uint32_t ms = 0;

void SysTick_Handler(void) { ms++; }

int main(void)
{
    SCG0->FIRCCFG = SCG_FIRCCFG_FREQ_SEL(0b101); /* 96 MHz FIRC */
    SysTick_Config(96000);                         /* 1 ms tick   */

    serial_init(115200);
    display_open();

    display_put_string(0, 0, "display_drv OK", RGB_WHITE, RGB_BLACK);

    serial_init(115200); /* already inited, harmless second call */
    /* printf replacement — use serial directly to stay printf-free */

    while (1)
    {
        __WFI();
    }
}
