#include <arch/virtual_memory.h>
#include <kernel/kassert.h>
#include <kernel/memory.h>
#include <kernel/paging.h>
#include <kernel/usermode.h>
#include <libk/libk.h>
#include "segment.h"

void usermode_function()
{
	unsigned int i = 0;
	while (i < 100 * 1000)
		++i;

	/* __asm__ __volatile__ ( */
	/*              "mov $0, %%eax\n" */
	/*              "mov $0xdeadbeef, %%ebx\n" */
	/*              "mov $0x8BADF00D, %%ecx\n" */
	/*              "mov $0xBAAAAAAD, %%edx\n" */
	/*              "mov $0xBAADF00D, %%esi\n" */
	/*              "mov $0xBADDCAFE, %%edi\n" */
	/*              /1* "mov $0xCAFED00D, %%ebp\n" *1/ */
	/*              "int $0x80" */
	/*              : */
	/*              : */
	/*              : "eax", "ebx", "ecx", "edx", "esi", "edi" */
	/*              ); */

	while (1);
}

void usermode_entry(void* _args)
{
	void** args = _args;
	/* void* usermode_function = args[0]; */
	/* void* usermode_function_args = args[1]; */

	p_addr_t page = memory_page_frame_alloc();
	kassert(paging_map(page, 4096, VM_OPT_USER | VM_OPT_WRITE) == 0);

	memcpy((void*)4096, usermode_function, 65);
	const uint32_t esp = 4096 + 4096 - 4;

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
			  "i" (4096),
			  "r" (esp)
			: "eax", "memory");
}
