#ifndef MOLE_H
#define MOLE_H

#include <board.h>
#include <stdbool.h>

#include "tft_lcd.h"
#include "fonts.h"

void mole_init(void);
void mole_input_detect(void);
bool mole_game_tick(void);   /* returns true when time is up → start Simon Says */

#endif /* MOLE_H */
