#include <libk/libk.h>

void* memcpy(void* dest, const void* src, size_t size)
{
	char* dst = dest;
	const char* s = src;
	for ( ; size > 0; ++dst, ++s, --size)
		*dst = *s;

	return dest;
}
