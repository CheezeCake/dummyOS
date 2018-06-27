#ifndef _DUMMYOS_SYSCALL_H_
#define _DUMMYOS_SYSCALL_H_

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
#define NR_execve		11
#define NR_open			12
#define NR_close		13
#define NR_lseek		14
#define NR_read			15
#define NR_write		16
#define NR_setsid		17
#define NR_setpgid		18
#define NR_getpgid		19
#define NR_setpgrp		20
#define NR_getpgrp		21
#define NR_ioctl		22
#define NR_sbrk			23
#define NR_getdents		24

#define SYSCALL_NR_TOP		24 /**< last syscall number */
#define SYSCALL_NR_COUNT	(SYSCALL_NR_TOP + 1) /**< number of syscalls */

#endif
