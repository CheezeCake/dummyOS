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
int sys_execve(const char* __user path, char* const __user argv[],
			   char* const __user envp[]);
int sys_open(const char* __user path, int flags);
int sys_close(int fd);
off_t sys_lseek(int fd, off_t offset, int whence);
ssize_t sys_read(int fd, void* __user buf, size_t count);
ssize_t sys_write(int fd, const void* __user buf, size_t count);
pid_t sys_setsid(void);
int sys_setpgid(pid_t pid, pid_t pgid);
pid_t sys_getpgid(pid_t pid);
int sys_setpgrp(void);
pid_t sys_getpgrp(void);
int sys_ioctl(int fd, int request, intptr_t arg);
void* sys_sbrk(intptr_t increment);
ssize_t sys_getdents(int fd, struct dirent* __user dirp, size_t nbytes);


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
	[NR_execve]		= __syscall(sys_execve),
	[NR_open]		= __syscall(sys_open),
	[NR_close]		= __syscall(sys_close),
	[NR_lseek]		= __syscall(sys_lseek),
	[NR_read]		= __syscall(sys_read),
	[NR_write]		= __syscall(sys_write),
	[NR_setsid]		= __syscall(sys_setsid),
	[NR_setpgid]	= __syscall(sys_setpgid),
	[NR_getpgid]	= __syscall(sys_getpgid),
	[NR_setpgrp]	= __syscall(sys_setpgrp),
	[NR_getpgrp]	= __syscall(sys_getpgrp),
	[NR_ioctl]		= __syscall(sys_ioctl),
	[NR_sbrk]		= __syscall(sys_sbrk),
	[NR_getdents]	= __syscall(sys_getdents),
};

#include <kernel/log.h>
#include <kernel/sched/sched.h>
static int nosys(char* str)
{
	struct process* proc = sched_get_current_process();
	log_e_printf("pid=%d: %s\n", proc->pid, str);
	return 0;
}
