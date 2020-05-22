#ifndef _PORT_OXE9_
#define _PORT_OXE9_

#include "../io_ports.h"

#define IOPORT_0XE9 0xe9

int ioport_0xe9_puts(const char* str);

static inline void ioport_0xe9_putchar(char c)
{
	outb(IOPORT_0XE9, c);
}

#endif
