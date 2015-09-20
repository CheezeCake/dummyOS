#include <stdbool.h>
#include <kernel/kmalloc.h>
#include <kernel/kheap.h>
#include <kernel/panic.h>

#include <kernel/log.h>

/*
 * memory_block_t: header for memory_blocks
 * bit 0: used
 * bits 1 -> 31: size (the kernel virtual address space is 1GB,
 *  which means that we actually only need 30 bits to store the size of a block
 *  from this address space, but since we typedef memory_block to uint32_t,
 *  we'll use the 31 bits left)
 */
typedef uint32_t memory_block_t;

static inline memory_block_t make_memory_block(size_t size, bool used)
{
	return ((size << 1) | (used ? 1 : 0));
}

#define memory_block_get_size(memory_block) (memory_block >> 1)
#define memory_block_get_used(memory_block) (memory_block & 1)

void kmalloc_init(p_addr_t kernel_top)
{
	size_t size = kheap_init(kernel_top, KHEAP_INITIAL_SIZE);
	if (size < KHEAP_INITIAL_SIZE)
		PANIC("not enough memory for kernel heap");

	log_printf("kheap_start = 0x%x, kheap_end = 0x%x, initial_size = 0x%x, "
			"heap_size_returned = 0x%x\n",
			kheap_get_start(), kheap_get_end(), KHEAP_INITIAL_SIZE, size);

	memory_block_t* kheap = (memory_block_t*)kheap_get_start();
	*kheap = make_memory_block(size - sizeof(memory_block_t), false);
}

void* kmalloc(size_t size)
{
	memory_block_t* current_block = (memory_block_t*)kheap_get_start();

	while ((v_addr_t)current_block < KHEAP_LIMIT &&
			(memory_block_get_size(*current_block) < size
			|| memory_block_get_used(*current_block))) {
		current_block = (memory_block_t*)
			((uint8_t*)current_block + sizeof(memory_block_t)
			 + memory_block_get_size(*current_block));
	}

	if ((v_addr_t)current_block >= KHEAP_LIMIT) {
		current_block = (memory_block_t*)kheap_sbrk(size);
		if (!current_block)
			return NULL;
	}

	size_t original_size = memory_block_get_size(*current_block);
	// is the block is large enough to be split?
	if (original_size - size > sizeof(memory_block_t)) {
		*current_block = make_memory_block(size, true);

		memory_block_t* second_part = (memory_block_t*)((uint8_t*)current_block
				+ sizeof(memory_block_t) + size);
		*second_part = make_memory_block(
				original_size - size - sizeof(memory_block_t), false);
	}
	else {
		*current_block |= 1; // set used bit
	}

	return (void*)((uint8_t*)current_block + sizeof(memory_block_t));
}

void kfree(void* ptr)
{
	if (!ptr || (v_addr_t)ptr < kheap_get_start() ||
			(v_addr_t)ptr > KHEAP_LIMIT)
		return;

	memory_block_t* block = (memory_block_t*)((uint8_t*)ptr - sizeof(memory_block_t));

	if (!memory_block_get_used(*block))
		return;

	*block |= 0; // unset used bit

	// try to merge with the next free blocks
	memory_block_t* next_block = (memory_block_t*)
		((uint8_t*)ptr + sizeof(memory_block_t) + memory_block_get_size(*block));
	memory_block_t* it = next_block;
	while ((v_addr_t)it < KHEAP_LIMIT && !memory_block_get_used(*it)) {
		it = (memory_block_t*)
			((uint8_t*)it + sizeof(memory_block_t) + memory_block_get_size(*block));
	}

	// merge
	if ((v_addr_t)it < KHEAP_LIMIT && it != next_block) {
		const size_t size = (size_t)((uint8_t*)it - ((uint8_t*)ptr + sizeof (memory_block_t)));
		*block = make_memory_block(size, false);
	}
}
