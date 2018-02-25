#include <kernel/types.h>
#include <kernel/usermode.h>

#include "segment.h"

void usermode_entry(v_addr_t entry_point, v_addr_t stack)
{
	__asm__ volatile ("movw %0, %%ax\n"
					  "movw %%ax, %%ds\n"
					  "movw %%ax, %%es\n"
					  "movw %%ax, %%fs\n"
					  "movw %%ax, %%gs\n"

					  "pushl %0\n" // ss
					  "pushl %3\n" // esp
					  "pushfl\n" // eflags
					  "pushl %1\n" // cs
					  "pushl %2\n" // eip
					  "iret"
					  :
					  : "i" (make_segment_selector(PRIVILEGE_USER, UDATA)),
						"i" (make_segment_selector(PRIVILEGE_USER, UCODE)),
						"r" (entry_point),
						"r" (stack)
					  : "eax", "memory");
}
