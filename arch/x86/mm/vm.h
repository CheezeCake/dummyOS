#ifndef _VM_H_
#define _VM_H_

#include <arch/memory.h>
#include <kernel/types.h>
#include <kernel/mm/vm.h>

/*
 * +---+------------------+ 4GB - 1 (0xffffffff)
 * |   |current           |
 * |   |recursive mapping |
 * |   +------------------+ 4GB - 4MB (0xffc00000)
 * |   |temp              |
 * |   |recursive mapping |
 * |   +------------------+ 4GB - 8MB (0xff800000)
 * |   +------------------+ 4GB - 256MB (0xf0000000)
 * |   | drivers, etc...  |
 * |   |                  |
 * |   +------------------+ 4GB - 512MB (0xe0000000)
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

#define RECURSIVE_ENTRY_START		0xffc00000 // 4GB - 4MB
#define KERNEL_SPACE_RESERVED		KERNEL_SPACE_START
#define TEMP_RECURSIVE_ENTRY_START	0xff800000 // 4GB - 8MB

/*
 * page directory entry count
 */
#define VM_COVERED_PER_PD_ENTRY		0x400000 // 4MB
#define KERNEL_SPACE_PD_ENTRIES					\
	(KERNEL_SPACE_SIZE / VM_COVERED_PER_PD_ENTRY) // 256
#define USER_SPACE_PD_ENTRIES					\
	(USER_SPACE_SIZE / VM_COVERED_PER_PD_ENTRY) // 768

#endif
