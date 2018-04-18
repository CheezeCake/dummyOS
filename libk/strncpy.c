#include <libk/libk.h>

char* strncpy(char* dest, const char* src, size_t size)
{
	char* dst = dest;
	for ( ; *src && size > 0; ++dest, ++src, --size)
		*dest = *src;

	// null pad
	for (; size > 0; ++dest, --size)
		*dest = '\0';

	return dst;
}
