/*! ***************************************************************************
 * \brief  Display HAL — portable header (Layer 3)
 * \file   display.h
 *
 * No MCU-specific includes. Any driver that implements these four functions
 * satisfies the HAL contract.
 *****************************************************************************/
#ifndef DISPLAY_H
#define DISPLAY_H

#include <stdint.h>

void display_open(void);
void display_clear(uint16_t color);
void display_put_string(uint16_t x, uint16_t y, const char *str,
                        uint16_t fg, uint16_t bg);
void display_close(void);

#endif /* DISPLAY_H */
