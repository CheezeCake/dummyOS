#include <libk/libk.h>

char* strcpy(char* dest, const char* src)
{
	char* dst = dest;
	for ( ; *src; ++src, ++dest)
		*dest = *src;

	return dst;
}
