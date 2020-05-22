#ifndef _KERNEL_MM_VM_H_
#define _KERNEL_MM_VM_H_

/*
 * address space sizes
 */
#define ADDR_SPACE_SIZE		0xffffffff // 4GB - 1
#define KERNEL_SPACE_SIZE	0x40000000 // 1GB
#define USER_SPACE_SIZE		0xc0000000 // 3GB

/*
 * kernel address space
 */
#define KERNEL_SPACE_START	0xc0000000 // 3GB
#define KERNEL_SPACE_END	0xffffffff // 4GB

/*
 * user address space
 */
#define USER_SPACE_START	0x0
#define USER_SPACE_END		0xc0000000 // 3GB

#endif
