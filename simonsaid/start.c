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

typedef enum{RED,GREEN,YELLOW,BLUE}color_e;
#define MIDDLE_X 2048
#define MIDDLE_Y 2048
#define QUARTER_X 2972
#define QUARTER_Y 2972
#define EDGE_X 3900
#define EDGE_Y 3900
#define PATTERN_SIZE 4
color_e gen_pattern[];
color_e user_input[]; 
static int i=0;
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

void input_detect(void){
if(lcd_touch_x > MIDDLE_X && lcd_touch_x < QUARTER_X && lcd_touch_y > MIDDLE_Y){//top right corner
    if(i==0){
        user_input[i]= RED;
    }
    else {user_input[i++]=RED;}
}
else if(lcd_touch_x > MIDDLE_X && lcd_touch_x < QUARTER_X && lcd_touch_y < MIDDLE_Y){//top left corner
	if(i==0){
        user_input[i]=GREEN;
    }
    else {user_input[i++]=GREEN;}
}
else if(lcd_touch_x > QUARTER_X && lcd_touch_x < EDGE_X && lcd_touch_y > MIDDLE_Y){//bottom left corner
	if(i==0){
        user_input[i]=YELLOW;
    }
    else{user_input[i++]=YELLOW;}
}
else if(lcd_touch_x > QUARTER_X && lcd_touch_x < EDGE_X && lcd_touch_y < MIDDLE_Y){//bottome right corner
    if(i==0){
        user_input[i]=BLUE;
    }
    else{user_input[i++]=BLUE;}
}
}

bool compare_pattern(void){
    
    for(int a=0,a < PATTERN_SIZE,a++)
    {
        if(user_input[a]==gen_pattern[a]){
            return true;
        }
        else{
            return false;
        }
    }
}

/*PATTERN GENERATOR UNDER CONSTRUCTION :))*/