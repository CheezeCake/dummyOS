#include <kernel/libk.h>

int memcmp(const void* s1, const void* s2, size_t size)
{
	const char* s01 = s1;
	const char* s02 = s2;
	while (size > 0 && *s01 == *s02) {
		++s01;
		++s02;
		--size;
	}

	return (size == 0) ? 0 :
		(*(const unsigned char*)s01 - *(const unsigned char*)s02);
}
