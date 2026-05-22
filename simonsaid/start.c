#include "start.h"
static const uint16_t color_lut[][2] =
    {
        {RGB_RED, RGB_BLACK},
        {RGB_GREEN, RGB_BLACK},
        {RGB_BLUE, RGB_BLACK},
        {RGB_YELLOW, RGB_BLACK},
        {RGB_CYAN, RGB_BLACK},
        {RGB_MAGENTA, RGB_BLACK},
        {RGB_WHITE, RGB_BLACK},
        {RGB_BLACK, RGB_WHITE},
        {RGB_GRAY, RGB_BLACK},
        {RGB_ORANGE, RGB_BLACK},
        {RGB_PURPLE, RGB_BLACK},
        {RGB_PINK, RGB_BLACK},
        {RGB_BROWN, RGB_BLACK},
        {RGB_LIME, RGB_BLACK},
        {RGB_NAVY, RGB_BLACK},
        {RGB_TEAL, RGB_BLACK},
};

static const uint32_t color_lut_size =
    sizeof(color_lut) / sizeof(color_lut[0]);

static uint8_t color_cnt = 0;

#define MIDDLE 85
void start_screen(void){
            color_cnt = (color_cnt + 1) % color_lut_size;
            lcd_set_font(Dialog_bold_16);
            lcd_orientation(ORIENTATION_90);
            uint8_t font_height = Dialog_bold_16[1];
            lcd_put_string(MIDDLE, 0 * font_height, "Simon Said game", RGB_WHITE, RGB_BLUE);
            lcd_set_font(Dialog_plain_12);
            font_height = Dialog_plain_12[1];
            lcd_put_string(0,2*font_height,"Instruction:",RGB_WHITE,RGB_BLACK);
            lcd_put_string(0,3*font_height,"See the partterns and repeat them.",RGB_WHITE,RGB_BLACK);
}
