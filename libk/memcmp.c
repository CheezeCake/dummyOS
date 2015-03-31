#include <kernel/libk.h>

int memcmp(const void* s1, const void* s2, size_t size)
{
	const unsigned char* s01 = s1;
	const unsigned char* s02 = s2;
	while (size > 0 && *s01 == *s02) {
		++s01;
		++s02;
		--size;
	}

	return (size == 0) ? 0 : (*s01 - *s02);
}
