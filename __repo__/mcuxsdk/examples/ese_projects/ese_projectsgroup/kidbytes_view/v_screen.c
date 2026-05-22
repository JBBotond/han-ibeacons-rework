#include "v_screen.h"
#include "display_drv.h"
#include "fonts.h"
#include <stdio.h>

/* X offset where the value text starts (after "Puzzle: ") */
#define VAL_X  80

void v_screen_draw(void)
{
    display_clear(RGB_BLACK);

    lcd_set_font(Dialog_bold_16);
    uint8_t h = Dialog_bold_16[1];

    display_put_string(0, 0,     "Room:", RGB_WHITE, RGB_BLACK);
    display_put_string(0, 2 * h, "Next:", RGB_WHITE, RGB_BLACK);
    display_put_string(0, 4 * h, "Puzzle:", RGB_WHITE, RGB_BLACK);
}

void v_screen_update(uint8_t room, uint8_t next, const char *puzzle_hint)
{
    char buf[16];
    lcd_set_font(Dialog_bold_16);
    uint8_t h = Dialog_bold_16[1];

    /* Overwrite value area with black then print new value */
    snprintf(buf, sizeof(buf), "%u   ", room);
    display_put_string(VAL_X, 0, buf, RGB_GREEN, RGB_BLACK);

    snprintf(buf, sizeof(buf), "%u   ", next);
    display_put_string(VAL_X, 2 * h, buf, RGB_CYAN, RGB_BLACK);

    snprintf(buf, sizeof(buf), "%-10s", puzzle_hint);
    display_put_string(VAL_X, 4 * h, buf, RGB_YELLOW, RGB_BLACK);
}
