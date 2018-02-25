#ifndef _KERNEL_EXEC_H_
#define _KERNEL_EXEC_H_

#include <dummyos/const.h>

int exec(const char* __kernel path, char* const __kernel argv[],
		 char* const __kernel envp[]);

#endif
