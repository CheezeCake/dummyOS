#ifndef _KMALLOC_H_
#define _KMALLOC_H_

#include <stddef.h>
#include <kernel/types.h>

void kmalloc_init(p_addr_t kernel_top);
void* kmalloc(size_t size);
void kfree(const void* ptr);

#endif
