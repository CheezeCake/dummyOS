#include <kernel/libk.h>

void* memset(void *s, int c, size_t size)
{
	char* s0 = s;
	for ( ; size > 0; ++s0, --size)
		*s0 = c;

	return s;
}
