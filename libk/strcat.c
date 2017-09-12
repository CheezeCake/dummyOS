#include <libk/libk.h>

char* strcat(char* str, const char* append)
{
	char* str0 = str;
	size_t len = strlen(str);

	for (str += len; *append; ++str, ++append)
		*str = *append;

	*str = '\0';

	return str0;
}
