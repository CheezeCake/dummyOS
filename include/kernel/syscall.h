#ifndef _KERNEL_SYSCALL_H_
#define _KERNEL_SYSCALL_H_

#define SYSCALL_NR_BASE 0 /**< first syscall number */

#define NR_nosys	0
#define NR_exit		1

#define SYSCALL_NR_TOP		0 /**< last syscall number */
#define SYSCALL_NR_COUNT	(SYSCALL_NR_TOP + 1) /**< number of syscalls */

#endif
