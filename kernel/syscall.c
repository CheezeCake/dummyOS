#include <kernel/process.h>
#include <kernel/syscall.h>
#include <kernel/types.h>

#define __syscall(s) ((v_addr_t)s)

/* static int nosys(void); */
static int nosys(char* str);
extern void sys_exit(int);
extern pid_t sys_fork(void);
extern pid_t sys_getpid(void);
extern pid_t sys_getppid(void);

v_addr_t syscall_table[SYSCALL_NR_COUNT] = {
	[NR_nosys]		= __syscall(nosys),
	[NR_exit]		= __syscall(sys_exit),
	[NR_fork]		= __syscall(sys_fork),
	[NR_getpid]		= __syscall(sys_getpid),
	[NR_getppid]	= __syscall(sys_getppid),
};

#include <kernel/log.h>
#include <kernel/sched/sched.h>
static int nosys(char* str)
{
	struct process* proc = sched_get_current_process();
	log_e_printf("pid=%d: %s\n", proc->pid, str);
	return 0;
}
