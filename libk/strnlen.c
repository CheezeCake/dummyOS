#include <libk/libk.h>

size_t strnlen(const char* str, size_t maxsize)
{
	const char* s = str;
	while (maxsize != 0 && *s) {
		++s;
		--maxsize;
	}

	return (s - str);
}
