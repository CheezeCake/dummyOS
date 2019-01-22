#include <kernel/process.h>
#include <dummyos/syscall.h>
#include <kernel/types.h>
#include <dummyos/stat.h>

static int nosys(void);
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
int sys_sigprocmask(int how, const sigset_t* set, sigset_t* oset);
int sys_pipe(int __user fds[2]);
int sys_dup(int oldfd);
int sys_dup2(int oldfd, int newfd);
int sys_stat(const char* __user path, struct stat* __user sb);
int sys_fstat(int fd, struct stat* __user sb);
int sys_nanosleep(const struct timespec* __user timeout,
		  struct timespec* __user remainder);

#define __syscall(s) ((v_addr_t)s)

v_addr_t syscall_table[_SYSCALL_NR_COUNT] = {
	[SYS_nosys]		= __syscall(nosys),
	[SYS_exit]		= __syscall(sys_exit),
	[SYS_fork]		= __syscall(sys_fork),
	[SYS_getpid]		= __syscall(sys_getpid),
	[SYS_getppid]		= __syscall(sys_getppid),
	[SYS_signal]		= __syscall(sys_signal),
	[SYS_sigaction]		= __syscall(sys_sigaction),
	[SYS_sigreturn]		= __syscall(sys_sigreturn),
	[SYS_kill]		= __syscall(sys_kill),
	[SYS_wait]		= __syscall(sys_wait),
	[SYS_waitpid]		= __syscall(sys_waitpid),
	[SYS_execve]		= __syscall(sys_execve),
	[SYS_open]		= __syscall(sys_open),
	[SYS_close]		= __syscall(sys_close),
	[SYS_lseek]		= __syscall(sys_lseek),
	[SYS_read]		= __syscall(sys_read),
	[SYS_write]		= __syscall(sys_write),
	[SYS_setsid]		= __syscall(sys_setsid),
	[SYS_setpgid]		= __syscall(sys_setpgid),
	[SYS_getpgid]		= __syscall(sys_getpgid),
	[SYS_setpgrp]		= __syscall(sys_setpgrp),
	[SYS_getpgrp]		= __syscall(sys_getpgrp),
	[SYS_ioctl]		= __syscall(sys_ioctl),
	[SYS_sbrk]		= __syscall(sys_sbrk),
	[SYS_getdents]		= __syscall(sys_getdents),
	[SYS_sigprocmask]	= __syscall(sys_sigprocmask),
	[SYS_pipe]		= __syscall(sys_pipe),
	[SYS_dup]		= __syscall(sys_dup),
	[SYS_dup2]		= __syscall(sys_dup2),
	[SYS_stat]		= __syscall(sys_stat),
	[SYS_fstat]		= __syscall(sys_fstat),
	[SYS_nanosleep]		= __syscall(sys_nanosleep),
};

static int nosys(void)
{
	return 0;
}
