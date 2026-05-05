/*! ***************************************************************************
 * \brief  KidBytes View — Step 5 test
 * \file   main.c
 *
 * Test: toggles LOW BAT and FAULT every 2 s. RSSI + progress still cycle.
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
    v_screen_update(1, 2, "SUDOKU");
    v_icons_rssi(3);
    v_icons_progress(1);

    uint8_t toggle = 0;
    uint32_t next_tick = ms + 2000;

    while (1)
    {
        if ((int32_t)(ms - next_tick) >= 0)
        {
            next_tick += 2000;
            toggle ^= 1;

            v_icons_set_battery_low(toggle);
            v_icons_set_fault(!toggle);
        }
        __WFI();
    }
}
