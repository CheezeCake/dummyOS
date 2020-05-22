#include <dummyos/errno.h>
#include <kernel/kassert.h>
#include <kernel/kernel_image.h>
#include <kernel/mm/vmm.h>
#include <libk/libk.h>
#include "paging.h"

#include <kernel/log.h>

// defined in start.S
extern uint64_t __boot_ttbr1_lv2;
extern uint64_t __boot_ttbr1_lv2_phys;

static uint64_t* ttbr1_lv2 = &__boot_ttbr1_lv2;

#define vaddr2lv1_index(vaddr) ((vaddr >> 30) & 0x3)
#define vaddr2lv2_index(vaddr) ((vaddr >> 21) & 0x1ff)
#define vaddr2lv3_index(vaddr) ((vaddr >> 12) & 0x1ff)

#define ttbr0_lv1_2_3_idx2_vaddr(lv1_idx, lv2_idx, lv3_idx)	\
	((lv1_idx << 29) | (lv2_idx << 21) | (lv3_idx << 12))
#define ttbr1_lv2_3_idx2v_addr(lv2_idx, lv3_idx)		\
	(0xc0000000 | (lv2_idx << 21) | (lv3_idx << 12))

static inline p_addr_t descriptor_address(uint64_t desc)
{
	return (desc & 0xfffff000);
}

static inline bool valid_descriptor(uint64_t desc)
{
	return (desc & 1);
}

static inline uint64_t make_lv3_descriptor(p_addr_t addr, int prot)
{
	const unsigned desc_prot = ((prot & VMM_PROT_USER) ? AP_USER : AP_KERNEL) |
		((prot & VMM_PROT_WRITE) ? AP_RW : AP_RO);
	const unsigned attr = (prot & VMM_PROT_NOCACHE) ? ATTR_DEV : ATTR_MEM;

	return (addr | AF | desc_prot | attr | DESC_PAGE);
}

static inline uint64_t make_lv1_2_descriptor(p_addr_t addr)
{
	return make_lv3_descriptor(addr, VMM_PROT_WRITE);
}

static inline uint64_t make_lv1_descriptor(p_addr_t addr)
{
	return make_lv1_2_descriptor(addr);
}

static inline uint64_t make_lv2_descriptor(p_addr_t addr)
{
	return make_lv1_2_descriptor(addr);
}

static inline void lv3_descriptor_update_prot(uint64_t* desc, int prot)
{
	*desc = make_lv3_descriptor(descriptor_address(*desc), prot);
}

static inline void lv3_descriptor_set_ro(uint64_t* desc)
{
	*desc |= AP_RO;
}

static inline void invalidate_tlb_entry(v_addr_t addr)
{
	// TLBIMVAA (invalidate unified TLB by MVA, all ASID)
	__asm__ volatile ("mcr p15, #0, %0, c8, c7, #3" : : "r" (addr) : "memory");
}

#define TTBR1_RECURSIVE_LV2_ENTRY (LV2_ENTRIES - 2) // high exception vectors need the last entry
#define TTBR1_LV2_FREE_ENTRIES_END (LV2_ENTRIES - 3)

static uint64_t* get_ttbr1_lv3(unsigned lv2_idx)
{
	return (uint64_t*)(0xc0000000 | (TTBR1_RECURSIVE_LV2_ENTRY << 21) | (lv2_idx << 12));
}

static void remap_kernel(void)
{
	const p_addr_t phys_top = kernel_image_get_top_page_frame();
	p_addr_t phys = kernel_image_get_base_page_frame();
	uint64_t* kernel_lv3 = get_ttbr1_lv3(0);
	unsigned i = 0;

	// map kernel with *proper* permissions
	while (phys < phys_top) {
		kassert(i < LV3_ENTRIES); // kernel size > 2MB?!
		kernel_lv3[i++] = make_lv3_descriptor(phys, VMM_PROT_WRITE);
		phys += PAGE_SIZE;
	}

	// clear other entries
	while (i < LV3_ENTRIES)
		kernel_lv3[i++] = 0;
}

int paging_init(void)
{
	// MAIR0
	uint32_t mair0 = (0xff << 0) | // Attr0: normal memory, RW
			 (0x04 << 8);  // Attr1: device memory
	__asm__ volatile ("mcr p15, #0, r1, c10, c2, #0" : : "r" (mair0));

	// setup recursive entry
	ttbr1_lv2[TTBR1_RECURSIVE_LV2_ENTRY] =
		make_lv2_descriptor((p_addr_t)&__boot_ttbr1_lv2_phys);
	invalidate_tlb_entry((v_addr_t)get_ttbr1_lv3(TTBR1_RECURSIVE_LV2_ENTRY));

	remap_kernel();

	return 0;
}

static void* map_page(p_addr_t paddr)
{
	int lv2_idx;
	size_t lv3_idx;

	for (lv2_idx = TTBR1_LV2_FREE_ENTRIES_END - 1; lv2_idx >= 0; --lv2_idx) {
		if (!valid_descriptor(ttbr1_lv2[lv2_idx])) {
			lv3_idx = 0;
			break;
		}

		uint64_t* lv3_table = get_ttbr1_lv3(lv2_idx);

		for (lv3_idx = 0;
		     lv3_idx < LV3_ENTRIES && valid_descriptor(lv3_table[lv3_idx]);
		     ++lv3_idx)
		{ }

		if (lv3_idx < LV3_ENTRIES)
			break;
	}
	if (lv2_idx < 0)
		return NULL;

	uint64_t* lv3_table = get_ttbr1_lv3(lv2_idx);

	if (!valid_descriptor(ttbr1_lv2[lv2_idx])) {
		p_addr_t lv3_frame = memory_page_frame_alloc();
		if (!lv3_frame)
			return NULL;

		ttbr1_lv2[lv2_idx] = make_lv2_descriptor(lv3_frame);
		invalidate_tlb_entry((v_addr_t)lv3_table);
		memset(lv3_table, 0, PAGE_SIZE);
	}

	lv3_table[lv3_idx] = make_lv3_descriptor(paddr, VMM_PROT_WRITE);

	return (void*)ttbr1_lv2_3_idx2v_addr(lv2_idx, lv3_idx);
}

static int unmap_page(void* page)
{
	size_t lv2_idx = vaddr2lv2_index((v_addr_t)page);
	size_t lv3_idx = vaddr2lv3_index((v_addr_t)page);
	uint64_t* lv3_table = get_ttbr1_lv3(lv2_idx);

	if (!valid_descriptor(lv3_table[lv3_idx]))
		return -EINVAL;

	lv3_table[lv3_idx] = 0;

	invalidate_tlb_entry((v_addr_t)page);

	return 0;
}

p_addr_t paging_virt_to_phys(v_addr_t vaddr)
{
	kassert(!vmm_is_userspace_address(vaddr));

	const size_t lv2_idx = vaddr2lv2_index(vaddr);
	const size_t lv3_idx = vaddr2lv3_index(vaddr);
	const uint64_t* lv3_table = get_ttbr1_lv3(lv2_idx);

	kassert(valid_descriptor(ttbr1_lv2[lv2_idx]) &&
		valid_descriptor(lv3_table[lv3_idx]));

	return descriptor_address(lv3_table[lv3_idx]) | (vaddr & 0xfff);
}

void paging_switch_ttbr0(p_addr_t ttbr0)
{
	log_printf("Switching ttbr0 (%p)!\n", (void*)ttbr0);
	__asm__ volatile ("mcr p15, #0, %0, c2, c0, #0" : : "r" (ttbr0) : "memory");

	// TLBIALL (invalidate entire unified TLB)
	__asm__ volatile ("mcr p15, #0, %0, c8, c7, #0" : : "r" (0) : "memory");
}


int paging_clone_current_cow(uint64_t* dst_lv1, uint64_t* src_lv1)
{
	for (size_t lv1_idx = 0; lv1_idx < LV1_ENTRIES; ++lv1_idx) {
		if (!valid_descriptor(src_lv1[lv1_idx]))
			continue;

		p_addr_t lv2_frame = memory_page_frame_alloc();
		if (!lv2_frame)
			return -ENOMEM;
		uint64_t* dst_lv2_table = map_page(lv2_frame);
		if (!dst_lv2_table) {
			memory_page_frame_free(lv2_frame);
			return -ENOMEM;
		}
		memset(dst_lv2_table, 0, PAGE_SIZE);
		dst_lv1[lv1_idx] = make_lv1_descriptor((p_addr_t)lv2_frame);
		uint64_t* src_lv2_table = map_page(descriptor_address(src_lv1[lv1_idx]));
		if (!src_lv2_table) {
			unmap_page(dst_lv2_table);
			return -ENOMEM;
		}

		for (size_t lv2_idx = 0; lv2_idx < LV2_ENTRIES; ++lv2_idx) {
			if (!valid_descriptor(src_lv2_table[lv2_idx]))
				continue;

			p_addr_t lv3_frame = memory_page_frame_alloc();
			if (!lv3_frame)
				return -ENOMEM;
			uint64_t* dst_lv3_table = map_page(lv3_frame);
			if (!dst_lv3_table) {
				memory_page_frame_free(lv3_frame);
				return -ENOMEM;
			}
			memset(dst_lv3_table, 0, PAGE_SIZE);
			dst_lv2_table[lv2_idx] = make_lv2_descriptor((p_addr_t)lv3_frame);
			uint64_t* src_lv3_table = map_page(descriptor_address(src_lv2_table[lv2_idx]));
			if (!src_lv3_table) {
				unmap_page(dst_lv3_table);
				return -ENOMEM;
			}

			for (size_t lv3_idx = 0; lv3_idx < LV3_ENTRIES; ++lv3_idx) {
				if (!valid_descriptor(src_lv3_table[lv3_idx]))
					continue;

				lv3_descriptor_set_ro(&src_lv3_table[lv3_idx]);
				dst_lv3_table[lv3_idx] = src_lv3_table[lv3_idx];

				invalidate_tlb_entry(ttbr0_lv1_2_3_idx2_vaddr(lv1_idx, lv2_idx, lv3_idx));
			}
		}
	}

	return 0;
}

static int clear_user_lv_table(uint64_t* table, size_t lv)
{
	const size_t entries[] = {
		0,
		LV1_ENTRIES,
		LV2_ENTRIES,
		LV3_ENTRIES,
	};
	int err = 0;

	kassert(lv >= 1 && lv <= 3);

	for (size_t i = 0; i < entries[lv]; ++i) {
		if (valid_descriptor(table[i])) {
			if (lv < 3) {
				p_addr_t sub_frame = descriptor_address(table[i]);
				uint64_t* sub_table = map_page(sub_frame);
				if (!sub_table) {
					err = -ENOMEM;
					continue;
				}

				clear_user_lv_table(sub_table, lv + 1);
				unmap_page(sub_table);
				memory_page_frame_free(sub_frame);
			}
			else {
				table[i] = 0;
			}
		}
	}

	return err;
}

int paging_clear_userspace(uint64_t* lv1_table)
{
	return clear_user_lv_table(lv1_table, 1);
}

int paging_copy_page(v_addr_t src_page, p_addr_t dst_frame)
{
	void* dst_page = map_page(dst_frame);

	if (!dst_page)
		return -ENOMEM;

	memcpy(dst_page, (void*)src_page, PAGE_SIZE);

	unmap_page(dst_page);

	return 0;
}

int paging_update_kernel_prot(v_addr_t vaddr, int prot)
{
	size_t lv2_idx = vaddr2lv2_index(vaddr);
	uint64_t* lv3_table = get_ttbr1_lv3(lv2_idx);
	size_t lv3_idx = vaddr2lv3_index(vaddr);

	if (!valid_descriptor(ttbr1_lv2[lv2_idx]) ||
	    !valid_descriptor(lv3_table[lv3_idx]))
		return -EINVAL;

	lv3_descriptor_update_prot(&lv3_table[lv3_idx], prot);
	invalidate_tlb_entry(vaddr);

	return 0;
}

int paging_update_user_prot(v_addr_t vaddr, int prot, uint64_t* lv1_table)
{
	size_t lv1_idx = vaddr2lv1_index(vaddr);
	size_t lv2_idx = vaddr2lv2_index(vaddr);
	size_t lv3_idx = vaddr2lv3_index(vaddr);
	int err = 0;

	if (!valid_descriptor(lv1_table[lv1_idx]))
		return -EINVAL;

	uint64_t* lv2_table = map_page(descriptor_address(lv1_table[lv1_idx]));
	if (!lv2_table)
		return -ENOMEM;
	if (!valid_descriptor(lv2_table[lv2_idx])) {
		err = -EINVAL;
		goto unmap_lv2_table;
	}

	uint64_t* lv3_table = map_page(descriptor_address(lv2_table[lv2_idx]));
	if (!lv3_table) {
		err = -ENOMEM;
		goto unmap_lv2_table;
	}
	if (!valid_descriptor(lv3_table[lv3_idx])) {
		err = -EINVAL;
		goto unmap_lv3_table;
	}

	lv3_descriptor_update_prot(&lv3_table[lv3_idx], prot);
	invalidate_tlb_entry(vaddr);

unmap_lv3_table:
	unmap_page(lv3_table);
unmap_lv2_table:
	unmap_page(lv2_table);

	return err;
}

int paging_kernel_map(p_addr_t paddr, v_addr_t vaddr, int prot)
{
	size_t lv2_idx = vaddr2lv2_index(vaddr);
	uint64_t* lv3_table = get_ttbr1_lv3(lv2_idx);
	size_t lv3_idx = vaddr2lv3_index(vaddr);

	if (valid_descriptor(ttbr1_lv2[lv2_idx])) {
		if (valid_descriptor(lv3_table[lv3_idx]))
			return -EADDRINUSE;
	}
	else {
		p_addr_t lv3_frame = memory_page_frame_alloc();
		if (!lv3_frame)
			return -ENOMEM;

		ttbr1_lv2[lv2_idx] = make_lv2_descriptor(lv3_frame);
		invalidate_tlb_entry((v_addr_t)lv3_table);

		memset(lv3_table, 0, PAGE_SIZE);
	}

	lv3_table[lv3_idx] = make_lv3_descriptor(paddr, prot);
	invalidate_tlb_entry(vaddr);

	return 0;
}

int paging_user_map(p_addr_t paddr, v_addr_t vaddr, int prot, uint64_t* lv1_table)
{
	size_t lv1_idx = vaddr2lv1_index(vaddr);
	size_t lv2_idx = vaddr2lv2_index(vaddr);
	size_t lv3_idx = vaddr2lv3_index(vaddr);
	int err = -ENOMEM;

	p_addr_t lv2_frame = 0;
	bool valid_lv1_entry = valid_descriptor(lv1_table[lv1_idx]);
	if (valid_lv1_entry) {
		lv2_frame = descriptor_address(lv1_table[lv1_idx]);
	}
	else {
		lv2_frame = memory_page_frame_alloc();
		if (!lv2_frame)
			return -ENOMEM;
		lv1_table[lv1_idx] = make_lv1_descriptor(lv2_frame);
	}

	uint64_t* lv2_table = map_page(lv2_frame);
	if (!lv2_table)
		goto clear_lv1_entry;
	if (!valid_lv1_entry)
		memset(lv2_table, 0, PAGE_SIZE);

	p_addr_t lv3_frame = 0;
	bool valid_lv2_entry = valid_descriptor(lv2_table[lv2_idx]);
	if (valid_lv2_entry) {
		lv3_frame = descriptor_address(lv2_table[lv2_idx]);
	}
	else {
		lv3_frame = memory_page_frame_alloc();
		if (!lv3_frame)
			goto unmap_lv2_table;
		lv2_table[lv2_idx] = make_lv2_descriptor(lv3_frame);
	}

	uint64_t* lv3_table = map_page(lv3_frame);
	if (!lv3_table)
		goto clear_lv2_entry;
	if (!valid_lv2_entry)
		memset(lv3_table, 0, PAGE_SIZE);

	if (valid_descriptor(lv3_table[lv3_idx])) {
		err = -EADDRINUSE;
		goto unmap_lv3_table;
	}

	lv3_table[lv3_idx] = make_lv3_descriptor(paddr, prot);
	invalidate_tlb_entry(vaddr);

	unmap_page(lv3_table);
	unmap_page(lv2_table);

	return 0;

unmap_lv3_table:
	unmap_page(lv3_table);
clear_lv2_entry:
	if (!valid_lv2_entry) {
		memory_page_frame_free(lv3_frame);
		lv2_table[lv2_idx] = 0;
	}
unmap_lv2_table:
	unmap_page(lv2_table);
clear_lv1_entry:
	if (!valid_lv1_entry) {
		memory_page_frame_free(lv2_frame);
		lv1_table[lv1_idx] = 0;
	}

	return err;
}

int paging_kernel_unmap(v_addr_t vaddr)
{
	size_t lv2_idx = vaddr2lv2_index(vaddr);
	uint64_t* lv3_table = get_ttbr1_lv3(lv2_idx);
	size_t lv3_idx = vaddr2lv3_index(vaddr);

	if (!valid_descriptor(ttbr1_lv2[lv2_idx]) ||
	    !valid_descriptor(lv3_table[lv3_idx]))
		return -EINVAL;

	lv3_table[lv3_idx] = 0;
	invalidate_tlb_entry(vaddr);

	return 0;
}

int paging_user_unmap(v_addr_t vaddr, uint64_t* lv1_table)
{
	size_t lv1_idx = vaddr2lv1_index(vaddr);
	size_t lv2_idx = vaddr2lv2_index(vaddr);
	size_t lv3_idx = vaddr2lv3_index(vaddr);
	int err = 0;

	if (!valid_descriptor(lv1_table[lv1_idx]))
		return -EINVAL;

	uint64_t* lv2_table = map_page(descriptor_address(lv1_table[lv1_idx]));
	if (!lv2_table)
		return -ENOMEM;
	if (!valid_descriptor(lv2_table[lv2_idx])) {
		goto unmap_lv2_table;
		err = -EINVAL;
	}

	uint64_t* lv3_table = map_page(descriptor_address(lv2_table[lv2_idx]));
	if (!lv3_table) {
		err = -ENOMEM;
		goto unmap_lv2_table;
	}
	if (!valid_descriptor(lv3_table[lv3_idx])) {
		goto unmap_lv3_table;
		err = -EINVAL;
	}

	lv3_table[lv3_idx] = 0;
	invalidate_tlb_entry(vaddr);

unmap_lv3_table:
	unmap_page(lv3_table);
unmap_lv2_table:
	unmap_page(lv2_table);

	return err;
}
