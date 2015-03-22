#include <kernel/libk.h>

char* strncat(char* str, const char* append, size_t count)
{
	char* str0 = str;
	size_t len = strlen(str);
	str += len;

	while (count > 0 && *append) {
		*str = *append;
		++str;
		++append;
	}

	*str = '\0';

	return str0;
}
