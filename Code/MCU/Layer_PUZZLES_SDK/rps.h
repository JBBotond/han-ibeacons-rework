#ifndef RPS_H
#define RPS_H

#include <board.h>
#include <stdio.h>
#include <stdbool.h>

#include "serial.h"
#include "tft_lcd.h"
#include "fonts.h"

void rps_init(void);
void rps_input_detect(void);
bool rps_game_tick(void);   /* returns true when player won → transition to next game */

#endif /* RPS_H */
