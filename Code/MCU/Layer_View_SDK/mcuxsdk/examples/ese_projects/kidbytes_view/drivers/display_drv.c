/*! ***************************************************************************
 * \brief  display_drv — MCXA153 implementation of display.h (Layer 4)
 * \file   display_drv.c
 *****************************************************************************/
#include "display_drv.h"
#include "fonts.h"

void display_open(void)
{
    lcd_init();
    lcd_orientation(ORIENTATION_90);   /* landscape: 320 wide x 240 tall */
    lcd_set_font(Dialog_plain_12);
    lcd_clear(RGB_BLACK);
}

void display_clear(uint16_t color)
{
    lcd_clear(color);
}

void display_put_string(uint16_t x, uint16_t y, const char *str,
                        uint16_t fg, uint16_t bg)
{
    lcd_put_string(x, y, str, fg, bg);
}

void display_close(void)
{
    /* no explicit shutdown needed */
}
