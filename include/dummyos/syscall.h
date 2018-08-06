#ifndef _DUMMYOS_SYSCALL_H_
#define _DUMMYOS_SYSCALL_H_

#define _SYSCALL_NR_BASE 0 /**< first syscall number */

#define SYS_nosys		0
#define SYS_exit		1
#define SYS_fork		2
#define SYS_getpid		3
#define SYS_getppid		4
#define SYS_signal		5
#define SYS_sigaction	6
#define SYS_sigreturn	7
#define SYS_kill		8
#define SYS_wait		9
#define SYS_waitpid		10
#define SYS_execve		11
#define SYS_open		12
#define SYS_close		13
#define SYS_lseek		14
#define SYS_read		15
#define SYS_write		16
#define SYS_setsid		17
#define SYS_setpgid		18
#define SYS_getpgid		19
#define SYS_setpgrp		20
#define SYS_getpgrp		21
#define SYS_ioctl		22
#define SYS_sbrk		23
#define SYS_getdents	24

#define _SYSCALL_NR_TOP		24 /**< last syscall number */
#define _SYSCALL_NR_COUNT	(_SYSCALL_NR_TOP + 1) /**< number of syscalls */

#endif
