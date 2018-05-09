#include <kernel/process.h>
#include <kernel/syscall.h>
#include <kernel/types.h>

/* static int nosys(void); */
static int nosys(char* str);
void sys_exit(int);
pid_t sys_fork(void);
pid_t sys_getpid(void);
pid_t sys_getppid(void);
sighandler_t sys_signal(uint32_t sig, sighandler_t handler);
int sys_sigaction(uint32_t sig, const struct sigaction* restrict __user act,
				  struct sigaction* restrict __user oact);
void sys_sigreturn(void);
int sys_kill(pid_t pid, uint32_t sig);
pid_t sys_wait(int* __user status);
pid_t sys_waitpid(pid_t pid, int* __user status, int options);


#define __syscall(s) ((v_addr_t)s)

v_addr_t syscall_table[SYSCALL_NR_COUNT] = {
	[NR_nosys]		= __syscall(nosys),
	[NR_exit]		= __syscall(sys_exit),
	[NR_fork]		= __syscall(sys_fork),
	[NR_getpid]		= __syscall(sys_getpid),
	[NR_getppid]	= __syscall(sys_getppid),
	[NR_signal]		= __syscall(sys_signal),
	[NR_sigaction]	= __syscall(sys_sigaction),
	[NR_sigreturn]	= __syscall(sys_sigreturn),
	[NR_kill]		= __syscall(sys_kill),
	[NR_wait]		= __syscall(sys_wait),
	[NR_waitpid]	= __syscall(sys_waitpid),
};

#include <kernel/log.h>
#include <kernel/sched/sched.h>
static int nosys(char* str)
{
	struct process* proc = sched_get_current_process();
	log_e_printf("pid=%d: %s\n", proc->pid, str);
	return 0;
}
