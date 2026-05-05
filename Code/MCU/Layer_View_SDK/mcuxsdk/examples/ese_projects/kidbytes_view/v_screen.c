#include "v_screen.h"
#include "display_drv.h"
#include "fonts.h"

void v_screen_draw(void)
{
    display_clear(RGB_BLACK);

    lcd_set_font(Dialog_bold_16);
    uint8_t h = Dialog_bold_16[1];

    display_put_string(0, 0,     "Room:", RGB_WHITE, RGB_BLACK);
    display_put_string(0, 2 * h, "Next:", RGB_WHITE, RGB_BLACK);
    display_put_string(0, 4 * h, "Puzzle:", RGB_WHITE, RGB_BLACK);
}

