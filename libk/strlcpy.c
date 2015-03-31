#include <kernel/libk.h>

size_t strlcpy(char* dest, const char* src, size_t size)
{
	const char* src0 = src;
	size_t s = size;

	for ( ; size > 1 && *src; ++dest, ++src, --size)
		*dest = *src;

	if (size == 0 || size == 1) {
		if (s != 0)
			*dest = '\0';

		while (*src)
			++src;
	}

	return (src - src0);
}
