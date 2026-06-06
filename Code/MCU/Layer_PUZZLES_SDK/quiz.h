#ifndef QUIZ_H
#define QUIZ_H

#include <board.h>
#include <stdbool.h>
#include "tft_lcd.h"
#include "fonts.h"

void quiz_init(void);
void quiz_input_detect(void);
bool quiz_game_tick(void);   /* returns true when all 3 correct → advance */

#endif /* QUIZ_H */
