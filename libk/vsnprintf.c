#include <kernel/libk.h>

#define PUTSTR(str)				\
	do {						\
		while (*str) {			\
			PUTCHAR(*(str++));	\
		}						\
	} while (0)

#define PRINT_UDECIDMAL(value, buffer)			\
	do {										\
		unsigned int n = 0;						\
		do {									\
			unsigned int m = value % 10;		\
												\
			buffer[n++] = '0' + m;				\
			value /= 10;						\
		} while (value != 0);					\
												\
		while (n > 0)							\
			PUTCHAR(buffer[--n]);				\
	} while (0)

int vsnprintf(char* str, size_t size, const char* format, va_list ap)
{
	char c;
	size_t pos = 0;
#define PUTCHAR(character) if (pos < size - 1) str[pos++] = character

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

						if (value < 0) {
							PUTCHAR('-');
							value = -value;
						}

						PRINT_UDECIDMAL(value, buffer);

						break;
					}
				case 'u':
				case 'l':
					if (format[1] == 'u' || c == 'u') {
							unsigned long value = va_arg(ap, unsigned long);
							char buffer[20];
							PRINT_UDECIDMAL(value, buffer);
							if (c != 'u')
								++format;
					}
					else if (format[1] == 'l' && format[2] == 'u') {
						unsigned long long value = va_arg(ap, unsigned long long);
						char buffer[20];
						PRINT_UDECIDMAL(value, buffer);
						format += 2;
					}
					break;
				case 'p':
					PUTCHAR('0');
					PUTCHAR('x');
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
						else {
							const char* null = "null";
							PUTSTR(null);
						}

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
