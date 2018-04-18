#include <kernel/syscall.h>
#include <kernel/types.h>

static int nosys(void);
extern void sys_exit(int);

v_addr_t syscall_table[SYSCALL_NR_COUNT] = {
	[NR_nosys] = (v_addr_t)nosys,
	[NR_exit] = (v_addr_t)sys_exit,
};

static int nosys(void)
{
	return 0;
}
