#include <kernel/libk.h>

char* strlcpy(char* dest, const char* src, size_t size)
{
	char* dst = dest;
	size_t s = size - 1;
	for ( ; size > 1; ++dest, ++src, --size)
		*dest = *src;

	dest[s] = '\0';

	return dst;
}
