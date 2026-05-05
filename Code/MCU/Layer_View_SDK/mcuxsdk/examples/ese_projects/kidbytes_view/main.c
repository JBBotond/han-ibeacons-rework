/*! ***************************************************************************
 * \brief  KidBytes View — Step 2 test
 * \file   main.c
 *
 * Test: screen shows static 3-line layout with placeholder text.
 *****************************************************************************/
#include <board.h>
#include "serial.h"
#include "display_drv.h"
#include "v_screen.h"

volatile uint32_t ms = 0;

void SysTick_Handler(void) { ms++; }

int main(void)
{
    SCG0->FIRCCFG = SCG_FIRCCFG_FREQ_SEL(0b101); /* 96 MHz FIRC */
    SysTick_Config(96000);                         /* 1 ms tick   */

    serial_init(115200);
    display_open();
    v_screen_draw();

    while (1)
    {
        __WFI();
    }
}
