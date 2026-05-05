#ifndef V_ICONS_H
#define V_ICONS_H

#include <stdint.h>
#include <stdbool.h>

void v_icons_rssi(uint8_t bars);        /* 0..4 bars */
void v_icons_progress(uint8_t k);       /* k out of 5 */
void v_icons_set_battery_low(bool on);
void v_icons_set_fault(bool on);

#endif /* V_ICONS_H */
