#include <dummyos/errno.h>
#include <kernel/kassert.h>
#include <kernel/kernel_image.h>
#include <kernel/kmalloc.h>
#include <kernel/log.h>
#include <kernel/mm/memory.h>
#include <kernel/page_frame_status.h>
#include <libk/libk.h>
#include <libk/list.h>

static LIST_DEFINE(mem_layout);

static mem_area_t* memory_layout_find_area(p_addr_t addr)
{
	list_node_t* it;

	list_foreach(&mem_layout, it) {
		mem_area_t* m = list_entry(it, mem_area_t, m_layout);
		if (addr >= m->start && addr <= m->end)
			return m;
	}

	return NULL;
}

static void memory_layout_insert_area(mem_area_t* area)
{
	list_node_t* it;

	list_foreach(&mem_layout, it) {
		mem_area_t* m = list_entry(it, mem_area_t, m_layout);
		if (m->start > m->end) {
			list_insert_before(it, &area->m_layout);
			return;
		}
	}

	list_push_back(&mem_layout, &area->m_layout);
}

void memory_layout_init(mem_area_t* layout, size_t n)
{
	for (size_t i = 0; i < n; ++i)
		memory_layout_insert_area(&layout[i]);
}

static void memory_layout_add_kernel(void)
{
	static mem_area_t first;
	mem_area_init(&first, 0, 0x1000, "null");
	memory_layout_insert_area(&first);

	static mem_area_t kernel;

	const p_addr_t kernel_base = kernel_image_get_base_page_frame();
	const p_addr_t kernel_top = kernel_image_get_top_page_frame();
	mem_area_init(&kernel, kernel_base, kernel_top, "kernel");

	memory_layout_insert_area(&kernel);
}


/*
 * describes a physical page
 */
struct page_frame
{
	p_addr_t addr;

	list_node_t pf_list;
};

static struct page_frame* page_frame_descriptors = NULL;

typedef struct pf_list_t
{
	list_t list;
	size_t n;
} pf_list_t;

static pf_list_t free_page_frames;
static pf_list_t used_page_frames;

static void memory_pf_list_init(pf_list_t* list)
{
	list_init(&list->list);
	list->n = 0;
}

static void memory_pf_list_insert(pf_list_t* list, struct page_frame* pf)
{
	list_push_back(&list->list, &pf->pf_list);
	++list->n;
}

static p_addr_t memory_base;
static p_addr_t memory_top;

p_addr_t memory_get_memory_base(void)
{
	return memory_base;
}

p_addr_t memory_get_memory_top(void)
{
	return memory_top;
}

static void memory_alloc_page_frame_descriptors(size_t ram_size_bytes)
{
	const size_t page_frame_descriptors_size =
		(ram_size_bytes >> PAGE_SIZE_SHIFT) * sizeof(struct page_frame);

	// allocate page_frame_descriptors with kmalloc_early
	// (allocated after kernel image)
	page_frame_descriptors =
		(struct page_frame*)kmalloc_early(page_frame_descriptors_size);
	kassert(page_frame_descriptors != NULL);
}

static void memory_init_constants(size_t ram_size_bytes)
{
	// top_memory_left = ram_size_bytes % PAGE_SIZE;
	const p_addr_t top_memory_left =
		(ram_size_bytes & ((1 << PAGE_SIZE_SHIFT) - 1));

	// start of memory at PAGE_SIZE B
	memory_base = PAGE_SIZE;
	memory_top = ram_size_bytes - top_memory_left;
}

void memory_init(size_t ram_size_bytes)
{
	const char free_label[] = "free";
	const char* label = NULL;
	const char* old_label = NULL;

	memory_alloc_page_frame_descriptors(ram_size_bytes);
	memory_init_constants(ram_size_bytes);
	memory_pf_list_init(&free_page_frames);
	memory_pf_list_init(&used_page_frames);

	memory_layout_add_kernel();

	struct page_frame* pf_descriptor = page_frame_descriptors;
	p_addr_t paddr = 0;

	while (paddr < memory_top)
	{
		pf_list_t* list;
		const mem_area_t* m = memory_layout_find_area(paddr);
		p_addr_t s;

		if (m) {
			kassert(page_is_aligned(m->start));
			kassert(page_is_aligned(m->end + 1));
			kassert(m->start < m->end);

			s = paddr;

			while (paddr < m->end + 1 && paddr < memory_top) {
				pf_descriptor->addr = paddr;
				memory_pf_list_insert(&used_page_frames, pf_descriptor++);

				paddr += PAGE_SIZE;
			}

			label = m->label;
		}
		else {
			s = paddr;

			pf_descriptor->addr = paddr;
			list = &free_page_frames;

			label = free_label;

			paddr += PAGE_SIZE;

			memory_pf_list_insert(list, pf_descriptor);
			++pf_descriptor;
		}

#ifndef NDEBUG
		if (label != old_label) {
			log_printf("[%p]\n %s\n", (void*)s, label);
			old_label = label;
		}
#endif
	}
}

static inline struct page_frame* get_page_frame_at(p_addr_t addr)
{
	// check if addr is PAGE_SIZE aligned
	// if it isn't, it's not a page frame address
	if (is_aligned(addr, PAGE_SIZE)) {
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

	list_erase(&pf->pf_list);
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
