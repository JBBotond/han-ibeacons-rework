/*! ***************************************************************************
 * \brief  KidBytes View — Step 6 test
 * \file   main.c
 *
 * Test: LED blinks once on each puzzle hint update (every 1 s).
 *****************************************************************************/
#include <board.h>
#include "serial.h"
#include "display_drv.h"
#include "v_screen.h"
#include "v_icons.h"
#include "v_fb.h"

volatile uint32_t ms = 0;

void SysTick_Handler(void) { ms++; }

int main(void)
{
    SCG0->FIRCCFG = SCG_FIRCCFG_FREQ_SEL(0b101); /* 96 MHz FIRC */
    SysTick_Config(96000);                         /* 1 ms tick   */

    serial_init(115200);
    v_fb_init();
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
            v_screen_update(room, room + 1, "SUDOKU");
            v_icons_rssi(idx % 5);
            v_icons_progress(room);

            /* LED blink on puzzle hint update */
            v_fb_puzzle_cue();

            idx++;
        }
        __WFI();
    }
}
