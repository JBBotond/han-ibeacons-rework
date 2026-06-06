/*! ***************************************************************************
 * \brief  kidbytes_puzzles — Layer 6/7 puzzle-plugin bench test
 * \file   main.c
 *
 * Test: drives Khanh's 4 games THROUGH the puzzle_plugin contract, exactly
 * as the FSM (Layer 5) would in the PUZZLE state. Proves each game can be
 * started, fed touch input, and polled for "solved" without the FSM knowing
 * which game it is.
 *
 * This is NOT Khanh's main.c — his active_game chaining is replaced by the
 * registry + the same loop shape the KidBytes FSM uses. Touch advances each
 * puzzle; when a puzzle reports solved, we move to the next room's puzzle.
 *
 * Expected on serial (115200 bps):
 *   === kidbytes_puzzles — plugin contract bench test ===
 *   Room 0 puzzle: RPS    — started
 *   (play on TFT) ... [solved] → Room 1
 *   Room 1 puzzle: QUIZ   — started
 *   ...
 *
 * Hardware: TFT ILI9341 + XPT2046 touch over LPSPI (same wiring as tft_lcd).
 *****************************************************************************/
#include <board.h>
#include <stdio.h>

#include "serial.h"
#include "tft_lcd.h"
#include "lpspi_master.h"   /* touch_detected flag */
#include "fonts.h"
#include "puzzle_registry.h"

/* -------------------------------------------------------------------------
 * Globals (the games reference extern ms / touch_detected)
 * ---------------------------------------------------------------------- */
volatile uint32_t ms = 0;

void SysTick_Handler(void) { ms++; }

/* -------------------------------------------------------------------------
 * Main — mirrors the FSM PUZZLE-state loop, one room at a time
 * ---------------------------------------------------------------------- */
int main(void)
{
    /* 96 MHz FIRC */
    SCG0->FIRCCFG = SCG_FIRCCFG_FREQ_SEL(0b101);
    SysTick_Config(96000);  /* 1 ms tick */

    serial_init(115200);
    lcd_init();
    lcd_clear(RGB_BLACK);
    lcd_orientation(ORIENTATION_90);

    printf("\r\n=== kidbytes_puzzles — plugin contract bench test ===\r\n");
    printf("Build %s %s\r\n", __DATE__, __TIME__);
    printf("Puzzles registered: %u\r\n\r\n", puzzle_registry_count());

    uint8_t room = 0;
    const puzzle_plugin_t *p = puzzle_registry_get(room);

    printf("Room %u puzzle: %s — started\r\n", room, p->name);
    p->start();

    while (1)
    {
        __WFI();

        /* Feed touch into the active puzzle (Layer 5 would post a button/
         * touch event; here we call input directly when touch fired). */
        if (touch_detected)
        {
            touch_detected = false;
            lcd_get_touch();
            p->input();
        }

        /* Poll the puzzle: solved? (this is F2.2 — advance on solve) */
        if (p->tick())
        {
            printf("Room %u puzzle: %s — [SOLVED]\r\n", room, p->name);

            room++;
            if (room >= puzzle_registry_count())
            {
                printf("\r\nAll puzzles solved! (FSM would go FINAL → unlock)\r\n");
                lcd_clear(RGB_BLACK);
                lcd_orientation(ORIENTATION_90);
                lcd_set_font(Dialog_bold_16);
                lcd_put_string(60, 110, "ALL DONE!", RGB_GREEN, RGB_BLACK);
                while (1) { __WFI(); }
            }

            p = puzzle_registry_get(room);
            printf("Room %u puzzle: %s — started\r\n", room, p->name);
            p->start();
        }
    }
}
