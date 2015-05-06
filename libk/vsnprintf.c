#include <kernel/libk.h>

int vsnprintf(char* str, size_t size, const char* format, va_list ap)
{
#define PUTCHAR(character) if (pos < size - 1) str[pos++] = character
	char c;
	size_t pos = 0;

	for (; *format; ++format) {
		c = *format;

		if (c == '%') {
			switch ((c = *(++format))) {
				case '%':
					PUTCHAR(c);
					break;
				case 'c':
					PUTCHAR((unsigned char)va_arg(ap, int));
					break;
				case 'i':
				case 'd':
					{
						int value = va_arg(ap, int);
						char buffer[10];
						int n = 0;

						if (value < 0)
							PUTCHAR('-');

						do {
							int m = value % 10;
							if (m < 0)
								m = -m;

							buffer[n++] = '0' + m;
							value /= 10;
						} while (value != 0);

						while (n > 0)
							PUTCHAR(buffer[--n]);

						break;
					}
				case 'x':
				case 'X':
					{
						unsigned int value = va_arg(ap, int);
						char buffer[8];
						int n = 0;

						do {
							int m = value % 16;
							char hex_char = (m <= 9) ? ('0' + m) : ('a' + m - 10);

							buffer[n++] = hex_char;
							value >>= 4;
						} while (value != 0);

						while (n > 0)
							PUTCHAR(buffer[--n]);

						break;
					}
				case 's':
					{
						char* s = va_arg(ap, char*);
						if (s) {
							for (; *s; ++s)
								PUTCHAR(*s);
						}
						// else, print "null" ?

						break;
					}
				default:
					PUTCHAR('%');
					PUTCHAR(c);
					break;
			}
		}
		else {
			PUTCHAR(c);
		}
	}

	str[pos] = '\0';

	return pos;
}
