/*! ***************************************************************************
 * \brief  KidBytes View — Step 4 test
 * \file   main.c
 *
 * Test: cycles RSSI 0→4 bars and room 1→5 every 500 ms.
 *****************************************************************************/
#include <board.h>
#include "serial.h"
#include "display_drv.h"
#include "v_screen.h"
#include "v_icons.h"

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
    uint32_t next_tick = ms + 500;

    while (1)
    {
        if ((int32_t)(ms - next_tick) >= 0)
        {
            next_tick += 500;

            uint8_t room = (idx % 5) + 1;
            uint8_t rssi = idx % 5;  /* 0..4 */

            v_screen_update(room, room + 1, "SUDOKU");
            v_icons_rssi(rssi);
            v_icons_progress(room);

            idx++;
        }
        __WFI();
    }
}
