/*! ***************************************************************************
 * \brief  KidBytes View — Step 3 test
 * \file   main.c
 *
 * Test: cycles room=1..5, next=2..5, puzzle_hint="SUDOKU" every 1 s.
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

    uint8_t idx = 0;
    uint32_t next_tick = ms + 1000;

    while (1)
    {
        if ((int32_t)(ms - next_tick) >= 0)
        {
            next_tick += 1000;
            uint8_t room = (idx % 5) + 1;
            uint8_t next = (idx % 4) + 2;
            v_screen_update(room, next, "SUDOKU");
            idx++;
        }
        __WFI();
    }
}
