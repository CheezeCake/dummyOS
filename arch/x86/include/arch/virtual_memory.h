#ifndef _VIRTUAL_MEMORY_H_
#define _VIRTUAL_MEMORY_H_

#define KERNEL_VADDR_SPACE_TOP 0x40000000 // 1GB
#define MIRRORING_VADDR_BEGIN (KERNEL_VADDR_SPACE_TOP - 0x1000000) // 1GB - 4MB

#endif