#include <kernel/libk.h>

int strncmp(const char* s1, const char* s2, size_t size)
{
	while (size > 0) {
		if (*s1 != *s2++)
			return (*(const unsigned char*)s1 -
					*(const unsigned char*)(s2 - 1));

		if (*s1++ == '\0')
			return 0;

		--size;
	}

	return 0;
}
