#include "ioport_0xe9.h"

int ioport_0xe9_puts(const char* str)
{
	int len;
	for (len = 0; str[len]; len++)
		ioport_0xe9_putchar(str[len]);

	return len;
}
