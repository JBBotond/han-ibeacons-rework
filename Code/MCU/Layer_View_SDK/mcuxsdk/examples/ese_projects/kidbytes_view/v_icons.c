#include "v_icons.h"
#include "display_drv.h"
#include "fonts.h"
#include <stdio.h>

/* RSSI bars: top-right corner, 4 bars with increasing height */
#define BAR_W       6
#define BAR_GAP     3
#define BAR_MAX_H   20
#define BAR_BASE_Y  4           /* top margin */
#define BAR_RIGHT_X (lcd_width - 4)  /* right margin */

static void fill_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color)
{
    uint32_t total = (uint32_t)w * h;
    /* Fill framebuffer in chunks */
    uint32_t chunk = LCD_FRAMEBUFFER_WIDTH * LCD_FRAMEBUFFER_HEIGHT;
    for (uint32_t i = 0; i < chunk && i < total; i++)
        lcd_framebuffer[i] = color;

    lcd_set_area(x, y, w, h);
    lcd_write_pixels(lcd_framebuffer, total);
}

void v_icons_rssi(uint8_t bars)
{
    if (bars > 4) bars = 4;

    for (uint8_t i = 0; i < 4; i++)
    {
        uint16_t bar_h = (BAR_MAX_H / 4) * (i + 1);  /* 5,10,15,20 */
        uint16_t x = BAR_RIGHT_X - (4 - i) * (BAR_W + BAR_GAP);
        uint16_t y = BAR_BASE_Y + (BAR_MAX_H - bar_h);
        uint16_t color = (i < bars) ? RGB_GREEN : RGB_GRAY;

        fill_rect(x, y, BAR_W, bar_h, color);
    }
}

void v_icons_progress(uint8_t k)
{
    char buf[8];
    if (k > 5) k = 5;
    snprintf(buf, sizeof(buf), "%u/5 ", k);

    lcd_set_font(Dialog_bold_16);
    /* Place below RSSI bars, right-aligned area */
    uint16_t x = lcd_width - 50;
    uint16_t y = BAR_BASE_Y + BAR_MAX_H + 4;
    display_put_string(x, y, buf, RGB_WHITE, RGB_BLACK);
}

/* Bottom-left: "LOW BAT" label */
#define BAT_X  0
#define BAT_Y  (lcd_height - 16)

void v_icons_set_battery_low(bool on)
{
    lcd_set_font(Dialog_plain_12);
    if (on)
        display_put_string(BAT_X, BAT_Y, "LOW BAT", RGB_RED, RGB_BLACK);
    else
        display_put_string(BAT_X, BAT_Y, "       ", RGB_BLACK, RGB_BLACK);
}

/* Bottom-right: "FAULT" label */
#define FAULT_X (lcd_width - 60)
#define FAULT_Y (lcd_height - 16)

void v_icons_set_fault(bool on)
{
    lcd_set_font(Dialog_plain_12);
    if (on)
        display_put_string(FAULT_X, FAULT_Y, "FAULT", RGB_YELLOW, RGB_RED);
    else
        display_put_string(FAULT_X, FAULT_Y, "     ", RGB_BLACK, RGB_BLACK);
}
