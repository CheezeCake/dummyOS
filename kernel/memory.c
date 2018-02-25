#include <kernel/errno.h>
#include <kernel/kassert.h>
#include <kernel/kernel_image.h>
#include <kernel/kmalloc.h>
#include <kernel/log.h>
#include <kernel/memory.h>
#include <kernel/page_frame_status.h>
#include <libk/libk.h>
#include <libk/list.h>

/*
 * describes a physical page
 */
struct page_frame
{
	p_addr_t addr;

	list_node_t pf_list;
};

static struct page_frame* page_frame_descriptors = NULL;

typedef struct pf_list_t {
	list_t list;
	size_t n;
} pf_list_t;

static pf_list_t free_page_frames;
static pf_list_t used_page_frames;

static p_addr_t memory_base;
static p_addr_t memory_top;

void memory_init(size_t ram_size_bytes)
{
	const size_t page_frame_descriptors_size =
		(ram_size_bytes >> PAGE_SIZE_SHIFT) * sizeof(struct page_frame);

	// allocate page_frame_descriptors with kmalloc_early (allocated after kernel image)
	page_frame_descriptors = (struct page_frame*)kmalloc_early(page_frame_descriptors_size);
	kassert(page_frame_descriptors != NULL);

	const p_addr_t kernel_base = kernel_image_get_base_page_frame();
	const p_addr_t kernel_top = kernel_image_get_top_page_frame();

	// top_memory_left = ram_size_bytes % PAGE_SIZE;
	const p_addr_t top_memory_left = (ram_size_bytes & ((1 << PAGE_SIZE_SHIFT) - 1));

	// start of memory at PAGE_SIZE B
	memory_base = PAGE_SIZE;
	memory_top = ram_size_bytes - top_memory_left;

	kassert(kernel_top < memory_top);

	list_init(&free_page_frames.list);
	free_page_frames.n = 0;
	list_init(&used_page_frames.list);
	free_page_frames.n = 0;

#ifndef NDEBUG
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
			if (status <= 3)
				log_printf("[%p]\n %s\n", (void*)paddr, status_str[status]);
			else
				log_printf("[%p]\n %d\n", (void*)paddr, status);
		}
#endif

		memset(pf_descriptor, 0, sizeof(struct page_frame));
		pf_descriptor->addr = paddr;

		if (status == PAGE_FRAME_FREE) {
			list_push_back(&free_page_frames.list, &pf_descriptor->pf_list);
			++free_page_frames.n;
		}
		else if (status == PAGE_FRAME_KERNEL || status == PAGE_FRAME_HW_MAP) {
			list_push_back(&used_page_frames.list, &pf_descriptor->pf_list);
			++used_page_frames.n;
		}
	}
}

static inline struct page_frame* get_page_frame_at(p_addr_t addr)
{
	// check if addr is PAGE_SIZE aligned
	// if it isn't, it's not a page frame address
	if (IS_ALIGNED(addr, PAGE_SIZE)) {
		if (addr >= memory_base && addr < memory_top)
			return (page_frame_descriptors + (addr >> PAGE_SIZE_SHIFT));
	}

	return NULL;
}

p_addr_t memory_page_frame_alloc()
{
	// no free pages left
	if (list_empty(&free_page_frames.list))
		return (p_addr_t)NULL;

	struct page_frame* new_page_frame =
		list_entry(list_front(&free_page_frames.list), struct page_frame, pf_list);
	list_pop_front(&free_page_frames.list);
	--free_page_frames.n;

	list_push_back(&used_page_frames.list, &new_page_frame->pf_list);
	++used_page_frames.n;

	return new_page_frame->addr;
}

int memory_page_frame_free(p_addr_t addr)
{
	struct page_frame* pf = get_page_frame_at(addr);
	if (!pf)
		return -EINVAL;

	list_erase(&used_page_frames.list, &pf->pf_list);
	--used_page_frames.n;
	list_push_front(&free_page_frames.list, &pf->pf_list);
	++free_page_frames.n;

	return 0;
}

void memory_statistics(unsigned int* nb_used_page_frames,
		unsigned int* nb_free_page_frames)
{
	*nb_used_page_frames = used_page_frames.n;
	*nb_free_page_frames = free_page_frames.n;
}
