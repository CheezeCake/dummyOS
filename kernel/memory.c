#include <limits.h>
#include <kernel/memory.h>
#include <kernel/page_frame_status.h>
#include <kernel/libk.h>
#include <kernel/list.h>
#include <kernel/kassert.h>
#include <kernel/kernel_image.h>
#include <arch/memory.h>
#include <kernel/log.h>

static struct page_frame* page_frame_descriptors = NULL;

static struct page_frame_list free_page_frames = { NULL, 0 };
static struct page_frame_list used_page_frames = { NULL, 0 };

static p_addr_t memory_base;
static p_addr_t memory_top;

void memory_init(size_t ram_size_bytes)
{
	// place the frame descriptors right after the kernel
	page_frame_descriptors = (struct page_frame*)kernel_image_get_end();

	// shift kernel end by page_frame_descritors size
	const size_t page_frame_descriptors_size =
		(ram_size_bytes >> PAGE_SIZE_SHIFT) * sizeof(struct page_frame);
	kernel_image_shift_kernel_end(page_frame_descriptors_size);

	p_addr_t kernel_base = kernel_image_get_base_page_frame();
	p_addr_t kernel_top = kernel_image_get_top_page_frame();

	// top_memory_left = ram_size_bytes % PAGE_SIZE;
	p_addr_t top_memory_left = (ram_size_bytes & ((1 << PAGE_SIZE_SHIFT) - 1));

	// start of memory at PAGE_SIZE B
	memory_base = PAGE_SIZE;
	memory_top = ram_size_bytes - top_memory_left;

	kassert(kernel_top < memory_top);

#ifndef NDEBUG
	log_printf("kernel_base = 0x%x ; kernel_top = 0x%x\n", kernel_base, kernel_top);
	log_printf("memory_base = 0x%x ; memory_top = 0x%x\n", memory_base, memory_top);
	int stat = -1;
#endif

	struct page_frame* pf_descriptor = page_frame_descriptors;

	for (p_addr_t paddr = 0;
			paddr < ram_size_bytes;
			paddr += PAGE_SIZE, ++pf_descriptor) {
		enum page_frame_status status = get_page_frame_status(paddr,
				memory_base, memory_top, kernel_base, kernel_top);

#ifndef NDEBUG
		if (stat == -1 || status != stat) {
			const char* status_str[] = { "reserved", "kernel", "hw map", "free" };
			stat = status;
			if (status >= 0 && status <= 3)
				log_printf("[0x%x]\n %s\n", paddr, status_str[status]);
			else
				log_printf("[0x%x]\n %d\n", paddr, status);
		}
#endif

		memset(pf_descriptor, 0, sizeof(struct page_frame));
		pf_descriptor->addr = paddr;

		if (status == PAGE_FRAME_FREE) {
			list_push_back(&free_page_frames, pf_descriptor);
		}
		else if (status == PAGE_FRAME_KERNEL || status == PAGE_FRAME_HW_MAP) {
			list_push_back(&used_page_frames, pf_descriptor);
		}
	}
}

static inline struct page_frame* get_page_frame_at(p_addr_t addr)
{
	// check if addr is PAGE_SIZE aligned
	// if it isn't, it's not a page frame address
	if ((addr & ((1 << PAGE_SIZE_SHIFT) - 1)) != 0)
		return NULL;

	if (addr >= memory_base && addr < memory_top)
		return (page_frame_descriptors + (addr >> PAGE_SIZE_SHIFT));

	// addr is out of memory
	return NULL;
}

p_addr_t memory_page_frame_alloc()
{
	// no free pages left
	if (list_empty(&free_page_frames))
		return (p_addr_t)NULL;

	struct page_frame* new_page_frame = list_front(&free_page_frames);
	slist_pop_front(&free_page_frames);

	list_push_back(&used_page_frames, new_page_frame);

	return new_page_frame->addr;
}

int memory_page_frame_free(p_addr_t addr)
{
	struct page_frame* pf = get_page_frame_at(addr);
	if (!pf)
		return -1;

	list_erase(&used_page_frames, pf);
	list_push_front(&free_page_frames, pf);

	return 0;
}

void memory_statistics(unsigned int* nb_used_page_frames,
		unsigned int* nb_free_page_frames)
{
	*nb_used_page_frames = used_page_frames.size;
	*nb_free_page_frames = free_page_frames.size;
}
