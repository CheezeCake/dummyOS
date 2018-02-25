#include <kernel/syscall.h>

static int nosys(void);
extern void sys_exit(int);

void* syscall_table[SYSCALL_NR_COUNT] = {
	[NR_nosys] = nosys,
	[NR_exit] = sys_exit,
};

static int nosys(void)
{
	return 0;
}
