#ifndef V_SCREEN_H
#define V_SCREEN_H

#include <stdint.h>

void v_screen_draw(void);
void v_screen_update(uint8_t room, uint8_t next, const char *puzzle_hint);

#endif /* V_SCREEN_H */
