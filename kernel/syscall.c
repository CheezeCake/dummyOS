#include <stddef.h>

#include <kernel/syscall.h>

static int nosys(void);
extern void _exit(int);

void* syscall_table[SYSCALL_NR_COUNT] = {
	[NR_nosys] = nosys,
	[NR_exit] = _exit,
};

static int nosys(void)
{
	return 0;
}
