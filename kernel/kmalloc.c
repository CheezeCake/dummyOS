#include <stdbool.h>

#include <kernel/kmalloc.h>
#include <kernel/kheap.h>
#include <kernel/panic.h>
#include <kernel/locking/spinlock.h>
#include <kernel/kernel_image.h>
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

#define memory_block_get_header(ptr) \
	((memory_block_t*)((uint8_t*)ptr - sizeof(memory_block_t)))

#define memory_block_get_data(block_ptr) \
	((void*)((uint8_t*)block_ptr + sizeof(memory_block_t)))

#define memory_block_get_next(block_ptr) \
	((memory_block_t*)((uint8_t*)block_ptr + \
		sizeof(memory_block_t) + memory_block_get_size(*block_ptr)))

static bool kmalloc_init_done = false;
static spinlock_declare_lock(lock);
static memory_block_t* next_free_block = NULL;

void kmalloc_init(void)
{
	size_t size = kheap_init(kernel_image_get_top_page_frame(), KHEAP_INITIAL_SIZE);
	if (size < KHEAP_INITIAL_SIZE)
		PANIC("not enough memory for kernel heap");

	log_printf("kheap_start = 0x%x, kheap_end = 0x%x, initial_size = 0x%x, "
			"heap_size_returned = 0x%x | KHEAP_LIMIT = 0x%x\n",
			(unsigned int)kheap_get_start(), (unsigned int)kheap_get_end(),
			KHEAP_INITIAL_SIZE, (unsigned int)size, KHEAP_LIMIT);

	memory_block_t* kheap = (memory_block_t*)kheap_get_start();
	*kheap = make_memory_block(size - sizeof(memory_block_t), false);
	next_free_block = kheap;
	kmalloc_init_done = true;
}

static inline memory_block_t* __find_free_block(memory_block_t* start, v_addr_t end, size_t size)
{
	memory_block_t* current_block = start;

	while ((v_addr_t)current_block < end &&
			(memory_block_get_size(*current_block) < size
			 || memory_block_get_used(*current_block))) {
		current_block = memory_block_get_next(current_block);
	}

	return ((v_addr_t)current_block < end) ? current_block : NULL;
}

static memory_block_t* find_free_block(size_t size)
{
	// search for a free block between next_free_block and kheap_end
	memory_block_t* free_block = __find_free_block(next_free_block, kheap_get_end(), size);

	if (!free_block) {
		// search between start of kheap and next_free_block
		free_block = __find_free_block((memory_block_t*)kheap_get_start(),
				(v_addr_t)next_free_block, size);

		if (!free_block) {
			// increase kheap
			free_block = (memory_block_t*)kheap_sbrk(size);
			if (free_block == (memory_block_t*)-1)
				return NULL;
		}
	}

	return free_block;
}

void* kmalloc(size_t size)
{
	spinlock_lock(lock);

	memory_block_t* free_block = find_free_block(size);
	if (!free_block)
		return NULL;

	size_t original_size = memory_block_get_size(*free_block);
	// is the block is large enough to be split?
	if (original_size - size > sizeof(memory_block_t)) {
		*free_block = make_memory_block(size, true);

		memory_block_t* second_part =
			(memory_block_t*)(memory_block_get_data(free_block) + size);
		*second_part = make_memory_block(
				original_size - size - sizeof(memory_block_t), false);
	}
	else {
		*free_block |= 1; // set used bit
	}

	next_free_block = memory_block_get_next(free_block);

	spinlock_unlock(lock);

	return (void*)memory_block_get_data(free_block);
}

void kfree(void* ptr)
{
	if (!ptr)
		return;

	spinlock_lock(lock);

	const v_addr_t kheap_end = kheap_get_end();

	if (!ptr || (v_addr_t)ptr < kheap_get_start() ||
			(v_addr_t)ptr > kheap_end)
		return;

	memory_block_t* block = memory_block_get_header(ptr);

	if (!memory_block_get_used(*block))
		return;

	*block &= ~1; // unset used bit

	memory_block_t* it = block;
	memory_block_t* next_block = NULL;

	// try to merge with the next free blocks
	while ((v_addr_t)it < kheap_end && !memory_block_get_used(*it)) {
		next_block = it;
		it = memory_block_get_next(it);
	}

	// merge
	if ((v_addr_t)next_block < kheap_end && next_block != block) {
		const size_t size = (size_t)((uint8_t*)next_block - (uint8_t*)memory_block_get_data(block));
		*block = make_memory_block(size, false);
	}

	spinlock_unlock(lock);
}

p_addr_t kmalloc_early(size_t size)
{
	if (kmalloc_init_done)
		PANIC("kmalloc_early was called after the initialization of the kmalloc subsytem!");

	const p_addr_t kernel_end = kernel_image_get_end();
	kernel_image_shift_kernel_end(size);

	return kernel_end;
}
