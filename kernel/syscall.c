#include <stddef.h>

#include <kernel/syscall.h>

static int nosys(void);

void* syscall_table[SYSCALL_NR_COUNT] = {
	[NR_nosys] = nosys,
	/* [NR_exit] = NULL, */
};

static int nosys(void)
{
	return 0;
}
