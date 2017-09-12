#include <libk/libk.h>
#include "ioport_0xe9.h"

int ioport_0xe9_printf(const char* format, ...)
{
	const int buffer_size = 256;
	char buffer[buffer_size];
	va_list ap;

	va_start(ap, format);
	vsnprintf(buffer, buffer_size, format, ap);
	va_end(ap);

	return ioport_0xe9_puts(buffer);
}

int ioport_0xe9_puts(const char* str)
{
	int len;
	for (len = 0; str[len]; len++)
		ioport_0xe9_putchar(str[len]);

	return len;
}
