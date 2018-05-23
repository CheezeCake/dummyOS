#ifndef _KERNEL_EXEC_H_
#define _KERNEL_EXEC_H_

#include <kernel/types.h>

typedef struct user_arg
{
	char* str;
	size_t len;
} user_arg_t;

typedef struct user_args
{
	user_arg_t* args;
	size_t size;
} user_args_t;

#define USER_ARGS_EMPTY { .args = NULL, .size = 0 }

int exec(const char* path, const user_args_t* argv, const user_args_t* envp);

#endif
