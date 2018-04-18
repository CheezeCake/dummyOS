#include <kernel/kmalloc.h>
#include <libk/libk.h>

char* strdup(const char* str)
{
	size_t len = strlen(str);
	char* dup = kmalloc(len);

	if (dup)
		strncpy(dup, str, len);

	return dup;
}
