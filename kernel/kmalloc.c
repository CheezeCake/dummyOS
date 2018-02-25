#include <dummyos/compiler.h>
#include <kernel/kassert.h>
#include <kernel/kernel_image.h>
#include <kernel/kheap.h>
#include <kernel/kmalloc.h>
#include <kernel/locking/spinlock.h>
#include <kernel/log.h>
#include <kernel/panic.h>
#include <libk/utils.h>

#define MAGIC 0xc001b10c

// alocations are aligned to a 16-byte boundary
#define ALIGNMENT 16

/**
 * @brief Memory block header
 *
 * @note Should be 16 bytes on 32-bit systems and 32 bytes on 64-bit systems.
 * This means that if the header is aligned to a 16-byte boundary the
 * associated memory will be too.
 */
typedef struct memory_block_header
{
	uintptr_t magic;
	struct memory_block_header* next;

	size_t size;
	bool used;
} memory_block_header_t;

static_assert(sizeof(memory_block_header_t) % ALIGNMENT == 0,
			  "sizeof(memory_block_header_t) is not a multiple of ALIGNMENT");

static bool kmalloc_init_done = false;
static spinlock_t lock = SPINLOCK_NULL;

#ifdef DEBUG
static void dump(void)
{
	memory_block_header_t* block;

	for (block = (memory_block_header_t*)kheap_get_start();
		 block;
		 block = block->next)
	{
		log_i_printf("[%p: 0x%x bytes %s] ", block, block->size,
					 (block->used) ? "used" : "free");
	}
	log_i_putchar('\n');
}
#endif

static inline void make_memory_block(memory_block_header_t* ptr,
									 memory_block_header_t* next,
									 size_t size,
									 bool used)
{
	ptr->magic = MAGIC;
	ptr->next = next;
	ptr->size = size;
	ptr->used = used;
}

void kmalloc_init(v_addr_t kheap_start, size_t kheap_size)
{
	kassert(IS_ALIGNED(kheap_start, ALIGNMENT));

	memory_block_header_t* kheap = (memory_block_header_t*)kheap_start;
	make_memory_block(kheap, NULL, kheap_size, false);

	kmalloc_init_done = true;
}

void* kmalloc(size_t size)
{
	memory_block_header_t* block;

	size = ALIGN_UP(size + sizeof(memory_block_header_t), ALIGNMENT);

	spinlock_lock(&lock);

	// find a free block
	block = (memory_block_header_t*)kheap_get_start();
	while (block->next && (block->size < size || block->used))
		block = block->next;

	// out of memory
	if (block->size < size || block->used) {
		size_t nr_pages = size / PAGE_SIZE;
		if (size % PAGE_SIZE > 0)
			++nr_pages;

		if (kheap_extend_pages(nr_pages) != nr_pages) {
			spinlock_unlock(&lock);
			return NULL;
		}

		memory_block_header_t* new = (void*)block + block->size;
		make_memory_block(new, NULL, nr_pages * PAGE_SIZE, false);

		block->next = new;

		block = new;
	}

	// is the block large enough to be split?
	if (block->size > size * 2) {
		memory_block_header_t* second_part = (void*)block + size;
		make_memory_block(second_part, block->next,
						  block->size - size, false);

		block->next = second_part;
		block->size = size;
	}

	block->used = true;

#ifdef DEBUG
	dump();
#endif

	spinlock_unlock(&lock);

	return (block + 1);
}

void kfree(void* ptr)
{
	if (!ptr)
		return;

	if (!IS_ALIGNED((v_addr_t)ptr, ALIGNMENT)) {
		log_i_printf("Trying to free invalid address! "
					 "%p is not %d-byte aligned.\n",
					 ptr, ALIGNMENT);
		return;
	}

	spinlock_lock(&lock);

	memory_block_header_t* block = (memory_block_header_t*)ptr - 1;

	if (!block->used) {
		log_i_printf("Trying to free a free block! (%p)\n", block);
		goto end;
	}
	if (block->magic != MAGIC) {
		log_i_printf("Trying to free an invalid block! "
					 "Invalid magic number (%p)\n", block);
		goto end;
	}

	memory_block_header_t* next;
	size_t merge_size = block->size;
	for (next = block->next; next && !next->used; next = next->next)
		merge_size += next->size;

	block->next = next;
	block->size = merge_size;
	block->used = false;

#ifdef DEBUG
	dump();
#endif

end:
	spinlock_unlock(&lock);
}

v_addr_t kmalloc_early(size_t size)
{
	if (kmalloc_init_done)
		PANIC("kmalloc_early was called after the initialization of the "
			  "kmalloc subsytem!");

	const v_addr_t kernel_end = kernel_image_get_virt_end();
	kernel_image_shift_kernel_end(size);

	return kernel_end;
}
