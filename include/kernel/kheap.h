#ifndef _KERNEL_KHEAP_H_
#define _KERNEL_KHEAP_H_

#include <arch/vm.h>
#include <kernel/types.h>

#define KHEAP_INITIAL_SIZE 0x80000 // 512kB

/**
 * @brief Initializes the kernel heap
 *
 * @param start the address where the heap will start
 * @param initial_size the initial size of the heap
 * @return the size of the initialized kernel heap
 * If the return value < initial_size => error
 *
 * @note start should be aligned to the required kmalloc alignment.
 * See kmalloc.c, ALIGNMENT is 16.
 */
size_t kheap_init(v_addr_t start, size_t initial_size);

/**
 * @brief Increases the heap size by a certain number of pages
 *
 * @param nr_pages the number of pages to add at the end of the heap
 * @return the number of pages added to the heap
 */
size_t kheap_extend_pages(size_t nr_pages);

/**
 * @brief Returns the starting address of the heap
 */
v_addr_t kheap_get_start(void);

/**
 * @brief Returns the ending address of the heap
 */
v_addr_t kheap_get_end(void);

#endif
