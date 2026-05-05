/*! ***************************************************************************
 * \brief  KidBytes View — Step 7 test
 * \file   main.c
 *
 * Test: Press SW2 → buzzer 200 ms + blue LED. Puzzle cue blinks green each 1s.
 *****************************************************************************/
#include <board.h>
#include "serial.h"
#include "switches.h"
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
    sw_init();
    v_fb_init();
    display_open();
    v_screen_draw();

    uint8_t idx = 0;
    uint32_t next_tick = ms + 1000;

    while (1)
    {
        /* SW2 pressed → unlock cue (buzzer + LED) */
        if (sw2_pressed())
        {
            v_fb_unlock_cue();
        }

        /* Periodic puzzle hint update with LED cue */
        if ((int32_t)(ms - next_tick) >= 0)
        {
            next_tick += 1000;
            uint8_t room = (idx % 5) + 1;
            v_screen_update(room, room + 1, "SUDOKU");
            v_icons_rssi(idx % 5);
            v_icons_progress(room);
            v_fb_puzzle_cue();
            idx++;
        }
        __WFI();
    }
}
