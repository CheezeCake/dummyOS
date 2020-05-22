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

static const mem_area_t* memory_layout_find_area(p_addr_t addr,
						 const mem_area_t* mem_layout,
						 size_t layout_len,
						 const mem_area_t* kernel_area)
{
	static const mem_area_t null = MEMORY_AREA(0, PAGE_SIZE, 0, "null");
	if (addr < PAGE_SIZE)
		return &null;
	if (addr >= kernel_area->start &&
	    addr < kernel_area->start + kernel_area->size)
		return kernel_area;

	ssize_t lo = 0;
	ssize_t hi = layout_len - 1;

	while (lo <= hi) {
		const ssize_t mid = (lo + hi) / 2;
		const p_addr_t start = mem_layout[mid].start;

		if (addr > start + mem_layout[mid].size)
			lo = mid + 1;
		else if (addr < start)
			hi = mid - 1;
		else
			return &mem_layout[mid];
	}

	return NULL;
}

/*
 * describes a physical page
 */
struct page_frame
{
	p_addr_t addr;

	list_node_t pf_list;
};

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

#define MEMORY_EARLY_FRAMES_MB 4
#define MEMORY_EARLY_FRAMES ((MEMORY_EARLY_FRAMES_MB * 1024 * 1024) / PAGE_SIZE)
static struct page_frame early_free_frames[MEMORY_EARLY_FRAMES];
static size_t early_free_frames_size = 0;

void __memory_early_init(void)
{
	p_addr_t paddr = kernel_image_get_top_page_frame();
	size_t i;

	memory_pf_list_init(&free_page_frames);
	memory_pf_list_init(&used_page_frames);

	for (i = 0; i < ARRAY_SIZE(early_free_frames); ++i, paddr += PAGE_SIZE) {
		early_free_frames[i].addr = paddr;
		memory_pf_list_insert(&free_page_frames, &early_free_frames[i]);
	}

	early_free_frames_size = i;
}

p_addr_t memory_get_memory_base(void)
{
	return memory_base;
}

p_addr_t memory_get_memory_top(void)
{
	return memory_top;
}

static size_t init_frames(p_addr_t base, p_addr_t top,
			  struct page_frame* descriptors,
			  const mem_area_t* mem_layout,
			  size_t layout_len,
			  const mem_area_t* kernel_area)
{
	p_addr_t paddr = base;
	size_t i = 0;

	while (paddr < top) {
		const mem_area_t* m = memory_layout_find_area(paddr,
							      mem_layout,
							      layout_len,
							      kernel_area);
		if (m) {
			while (paddr < m->start + m->size && paddr < top) {
				descriptors[i].addr = paddr;
				memory_pf_list_insert(&used_page_frames,
						      &descriptors[i++]);
				paddr += PAGE_SIZE;
			}
		}
		else {
			descriptors[i].addr = paddr;
			memory_pf_list_insert(&free_page_frames, &descriptors[i++]);
			paddr += PAGE_SIZE;
		}

	}

	return i;
}

static struct page_frame* page_frame_descriptors = NULL;

void memory_init(size_t ram_size_bytes, const mem_area_t* mem_layout,
		 size_t layout_len)
{
	kassert(MEMORY_EARLY_FRAMES_MB * 1024 * 1024 <= ram_size_bytes);
	const mem_area_t kernel_area = MEMORY_AREA(kernel_image_get_base_page_frame(),
						   kernel_image_get_size(),
						   0,
						   "kernel");

	memory_base = PAGE_SIZE;
	memory_top = page_align_down(ram_size_bytes);

	page_frame_descriptors = kcalloc(ram_size_bytes / PAGE_SIZE,
					 sizeof(struct page_frame));
	kassert(page_frame_descriptors != NULL);

	struct page_frame* descriptor = page_frame_descriptors;

	descriptor += init_frames(0, kernel_image_get_top_page_frame(),
				  descriptor,
				  mem_layout, layout_len, &kernel_area);

	for (size_t i = 0; i < early_free_frames_size; ++i)
		descriptor[i].addr = -1;
	descriptor += early_free_frames_size;

	init_frames(early_free_frames[early_free_frames_size - 1].addr + PAGE_SIZE,
		    memory_top,
		    descriptor,
		    mem_layout, layout_len, &kernel_area);
}

static inline struct page_frame* get_page_frame_at(p_addr_t addr)
{
	if (!is_aligned(addr, PAGE_SIZE) || addr < memory_base || addr >= memory_top)
		return NULL;

	struct page_frame* pf = &page_frame_descriptors[addr / PAGE_SIZE];

	if (pf->addr == -1) {
		p_addr_t start = early_free_frames[0].addr;
		return &early_free_frames[(addr - start) / PAGE_SIZE];
	}

	return pf;
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

	kassert(pf->addr == addr);

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
