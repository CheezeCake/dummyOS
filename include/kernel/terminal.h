#ifndef _TERMINAL_H_
#define _TERMINAL_H_

#include <stddef.h>
#include <stdint.h>

int terminal_printf(const char* format, ...)
	__attribute__((format(printf, 1, 2)));
void terminal_putchar(char c);
int terminal_puts(const char* str);
void terminal_write(const char* data, size_t size);
void terminal_clear(uint8_t color);
void terminal_init(void);

#endif
