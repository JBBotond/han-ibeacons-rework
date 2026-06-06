#ifndef V_SCREEN_H
#define V_SCREEN_H

#include <stdint.h>

void v_screen_draw(void);
void v_screen_update(uint8_t room, uint8_t next, const char *puzzle_hint);

/** Waypoint HOTTER/COLDER screen (no game info), driven by filtered RSSI. */
void v_screen_waypoint(int8_t rssi);

/** Force the next v_screen_waypoint() call to redraw (call when a waypoint
 *  hunt begins so the warmth word appears immediately). */
void v_screen_waypoint_reset(void);

#endif /* V_SCREEN_H */
