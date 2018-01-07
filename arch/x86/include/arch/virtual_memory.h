#ifndef _ARCH_VIRTUAL_MEMORY_H_
#define _ARCH_VIRTUAL_MEMORY_H_

#include <stdbool.h>

#include <arch/memory.h>

/*
 * +---+------------------+ 4GB - 1 (0xffffffff)
 * |   |                  |
 * |   |  mirroring zone  |
 * |   |                  |
 * |   +------------------+ 4GB - 1 - 4MB (0xffc00000)
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
#define VADDR_SPACE_SIZE		0xffffffff // 4GB - 1
#define KERNEL_VADDR_SPACE_SIZE 0x40000000 // 1GB
#define USER_VADDR_SPACE_SIZE	0xc0000000 // 3GB

/*
 * limits
 */
#define KERNEL_VADDR_SPACE_START	0xc0000000 // 3GB
#define KERNEL_VADDR_SPACE_END		0xffffffff // 4GB
#define MIRRORING_VADDR_BEGIN		0xffc00000 // 4GB - 1 - 4MB
#define KERNEL_VADDR_SPACE_LIMIT	MIRRORING_VADDR_BEGIN
#define KERNEL_VADDR_SPACE_RESERVED KERNEL_VADDR_SPACE_START

#define USER_VADDR_SPACE_START	0x40000000 // 1GB
#define USER_VADDR_SPACE_END	0xffffffff // 4GB

/*
 * page directory entry count
 */
#define VM_COVERED_PER_PD_ENTRY 0x400000 // 4MB
#define KERNEL_VADDR_SPACE_PAGE_DIRECTORY_ENTRIES \
	(KERNEL_VADDR_SPACE_SIZE / VM_COVERED_PER_PD_ENTRY) // 256
#define USER_VADDR_SPACE_PAGE_DIRECTORY_ENTRIES \
	(USER_VADDR_SPACE_SIZE / VM_COVERED_PER_PD_ENTRY) // 768

static inline bool virtual_memory_is_userspace_address(v_addr_t address)
{
	return (address < KERNEL_VADDR_SPACE_START);
}

#endif
