#include "tft_lcd.h"
#include "screens.h"

void lcd_new_line(char str[]) {
    line_count ++;
    if(line_count >= 12)
        line_count = 0;
    lcd_put_string(0, line_count * font_height, str, RGB_LIME, RGB_BLACK);
}

void lcd_reset_cursor(void) {
    lcd_clear(RGB_BLACK);
    line_count = 0;
}