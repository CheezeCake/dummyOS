#include <libk/libk.h>

size_t strlcpy(char* dest, const char* src, size_t size)
{
	const char* src0 = src;
	const size_t s = size;

	while (--size > 0 && (*dest++ = *src++))
		;

	if (size == 0) {
		if (s != 0)
			*dest = '\0';

		while (*src)
			++src;
	}

	return (src - src0);
}
