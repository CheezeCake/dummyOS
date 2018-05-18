#ifndef _KERNEL_EXEC_H_
#define _KERNEL_EXEC_H_

#include <kernel/types.h>

struct arg_str
{
	char* str;
	size_t len;
};

struct user_args
{
	struct arg_str* array;
	size_t size;
};

int exec(const char* path, const struct user_args* argv,
		 const struct user_args* envp);

#endif
