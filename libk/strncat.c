#include <kernel/libk.h>

char* strncat(char* str, const char* append, size_t count)
{
	char* str0 = str;
	size_t len = strlen(str);

	for (str += len; count > 0 && *append; ++str, ++append, --count)
		*str = *append;

	*str = '\0';

	return str0;
}
