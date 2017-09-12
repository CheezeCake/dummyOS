#include <libk/libk.h>

char* strncpy(char* dest, const char* src, size_t size)
{
	char* dst = dest;
	for ( ; size > 0; ++dest, ++src, --size)
		*dest = *src;

	return dst;
}
