#include <kernel/usermode.h>
#include <kernel/memory.h>
#include <kernel/paging.h>
#include <arch/virtual_memory.h>
#include <kernel/kassert.h>
#include <kernel/libk.h>
#include "segment.h"

void usermode_entry(void* _args)
{
	void** args = _args;
	void* usermode_function = args[0];
	/* void* usermode_function_args = args[1]; */

	p_addr_t page = memory_page_frame_alloc();
	kassert(paging_map(page, KERNEL_VADDR_SPACE_TOP, VM_OPT_USER | VM_OPT_WRITE) == 0);

	memcpy((void*)KERNEL_VADDR_SPACE_TOP, usermode_function, 176);
	const uint32_t esp = KERNEL_VADDR_SPACE_TOP + 4096 - 4;

	__asm__ __volatile__ (
			"movw %0, %%ax\n"
			"movw %%ax, %%ds\n"
			"movw %%ax, %%es\n"
			"movw %%ax, %%fs\n"
			"movw %%ax, %%gs\n"

			"pushl %0\n" // ss
			"pushl %3\n" // esp
			"pushfl\n" // eflags
			"pushl %1\n" // cs
			"pushl %2\n" // eip for user_mode_function
			"iret"
			:
			: "i" (make_segment_selector(PRIVILEGE_USER, UDATA)),
			  "i" (make_segment_selector(PRIVILEGE_USER, UCODE)),
			  "i" (KERNEL_VADDR_SPACE_TOP),
			  "r" (esp)
			: "eax", "memory");
}
