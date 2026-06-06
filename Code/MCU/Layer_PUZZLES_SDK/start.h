#ifndef START_H
#define START_H
#include <board.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "serial.h"
#include "tft_lcd.h"
#include "animations.h"
#include "bitmaps.h"
#include "fonts.h"

void start_screen(void);
void generate_pattern(void);
void input_detect(void);
void game_tick(void);

#endif /* START_H */
