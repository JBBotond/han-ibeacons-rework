/*
 * Serial Communication Module for FRDM-MCXA153
 * LPUART2 - Interrupt-driven TX/RX
 */

#ifndef SERIAL_COMMUNICATION_H
#define SERIAL_COMMUNICATION_H

#include <stdint.h>

void serial_init(uint32_t baudrate);
void serial_putchar(char c);
void serial_puts(const char *str);
uint32_t serial_available(void);
char serial_getchar(void);

#endif // SERIAL_COMMUNICATION_H
