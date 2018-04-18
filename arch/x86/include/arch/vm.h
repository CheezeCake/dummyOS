#ifndef _ARCH_VIRTUAL_MEMORY_H_
#define _ARCH_VIRTUAL_MEMORY_H_

#include <arch/memory.h>
#include <kernel/types.h>

/*
 * +---+------------------+ 4GB - 1 (0xffffffff)
 * |   |current           |
 * |   |recursive mapping |
 * |   +------------------+ 4GB - 4MB (0xffc00000)
 * |   |temp              |
 * |   |recursive mapping |
 * |   +------------------+ 4GB - 8MB (0xff800000)
 * |                      |
 * | kernel address space |
 * |                      |
 * |   +------------------+ 3GB + 0xa0000 (0xc00a0000)
 * |   |  reserved pages  |
 * +---+------------------+ 3GB (0xc0000000)
 * |                      |
 * |                      |
 * |                      |
 * |                      |
 * |                      |
 * |                      |
 * |                      |
 * |                      |
 * |  user address space  |
 * |                      |
 * |                      |
 * |                      |
 * |                      |
 * |                      |
 * |                      |
 * |                      |
 * |                      |
 * +----------------------+
 */


/*
 * address space sizes
 */
#define ADDR_SPACE_SIZE		0xffffffff // 4GB - 1
#define KERNEL_SPACE_SIZE	0x40000000 // 1GB
#define USER_SPACE_SIZE		0xc0000000 // 3GB

/*
 * limits
 */
#define KERNEL_SPACE_START	0xc0000000 // 3GB
#define KERNEL_SPACE_END		0xffffffff // 4GB
#define RECURSIVE_ENTRY_START		0xffc00000 // 4GB - 4MB
#define KERNEL_SPACE_LIMIT	RECURSIVE_ENTRY_START
#define KERNEL_SPACE_RESERVED KERNEL_SPACE_START

#define TEMP_RECURSIVE_ENTRY_START		0xff800000 // 4GB - 8MB

#define USER_SPACE_START	0x0
#define USER_SPACE_END	0xc0000000 // 3GB

/*
 * page directory entry count
 */
#define VM_COVERED_PER_PD_ENTRY 0x400000 // 4MB
#define KERNEL_SPACE_PD_ENTRIES \
	(KERNEL_SPACE_SIZE / VM_COVERED_PER_PD_ENTRY) // 256
#define USER_SPACE_PD_ENTRIES \
	(USER_SPACE_SIZE / VM_COVERED_PER_PD_ENTRY) // 768

#endif
