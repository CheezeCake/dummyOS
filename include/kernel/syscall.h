#ifndef _KERNEL_SYSCALL_H_
#define _KERNEL_SYSCALL_H_

#define SYSCALL_NR_BASE 0 /**< first syscall number */

#define NR_nosys		0
#define NR_exit			1
#define NR_fork			2
#define NR_getpid		3
#define NR_getppid		4
#define NR_signal		5
#define NR_sigaction	6
#define NR_sigreturn	7
#define NR_kill			8
#define NR_wait			9
#define NR_waitpid		10

#define SYSCALL_NR_TOP		10 /**< last syscall number */
#define SYSCALL_NR_COUNT	(SYSCALL_NR_TOP + 1) /**< number of syscalls */

#endif
