#include <libk/libk.h>
#include <kernel/types.h>

static inline void putchar(char* str, size_t size, size_t* pos, char c)
{
	if (*pos < size - 1)
		str[(*pos)++] = c;
}

static inline void print_unsigned_integer(char* str, size_t size, size_t* pos,
										  unsigned long long value, int base)
{
	static const char charset[] = "0123456789abcdef";
	char buffer[20];
	unsigned int n = 0;

	do {
		buffer[n++] = charset[value % base];
		value /= base;
	} while (value);

	while (n > 0)
		putchar(str, size, pos, buffer[--n]);
}

static inline void print_integer(char* str, size_t size, size_t* pos,
								 long long value)
{
	if (value < 0) {
		putchar(str, size, pos, '-');
		value = -value;
	}

	return print_unsigned_integer(str, size, pos, value, 10);
}

#define PUTSTR(str)			\
	while (*str) {			\
		PUTCHAR(*(str++));	\
	}

#define PRINT_UNSIGNED_INTEGER(type) __PRINT_UNSIGNED_INTEGER(type, 10)
#define PRINT_UNSIGNED_INTEGER_HEX(type) __PRINT_UNSIGNED_INTEGER(type, 16)

int vsnprintf(char* str, size_t size, const char* format, va_list ap)
{
	char c;
	size_t pos = 0;

#define PUTCHAR(character) putchar(str, size, &pos, character)
#define __PRINT_UNSIGNED_INTEGER(type, base)	\
	print_unsigned_integer(str, size, &pos, va_arg(ap, type), base)
#define PRINT_INTEGER(type) print_integer(str, size, &pos, va_arg(ap, type))

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
					PRINT_INTEGER(int);
					break;
				case 'u':
					PRINT_UNSIGNED_INTEGER(unsigned int);
					break;
				case 'l':
					if (format[1]) {
						// %l*
						if (format[1] == 'i' || format[1] == 'd') {
							PRINT_INTEGER(long);
						}
						else if (format[1] == 'u') {
							PRINT_UNSIGNED_INTEGER(unsigned long);
						}
						else if (format[1] == 'x' || format[1] == 'X') {
							PRINT_UNSIGNED_INTEGER_HEX(unsigned long);
						}
						else if (format[1] == 'l') {
							// %ll*
							if (format[2]) {
								if (format[2] == 'i' || format[2] == 'd')
									PRINT_INTEGER(long long);
								else if (format[2] == 'u')
									PRINT_UNSIGNED_INTEGER(unsigned long long);
								else if (format[2] == 'x' || format[2] == 'X')
									PRINT_UNSIGNED_INTEGER_HEX(unsigned long long);
							}
							++format;
						}
						++format;
					}
					break;
				case 'p':
					PUTCHAR('0');
					PUTCHAR('x');
					PRINT_UNSIGNED_INTEGER_HEX(v_addr_t);
					break;
				case 'x':
				case 'X':
					PRINT_UNSIGNED_INTEGER_HEX(unsigned int);
					break;
				case 's':
					{
						const char* s = va_arg(ap, char*);
						static const char* null = "(null)";
						if (!s)
							s = null;
						PUTSTR(s);
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
