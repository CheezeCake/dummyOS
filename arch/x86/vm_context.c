#include <kernel/vm_context.h>
#include <kernel/memory.h>
#include <arch/paging.h>
#include <kernel/paging.h>
#include <kernel/libk.h>

int vm_context_create(struct vm_context* vm_context)
{
	if ((vm_context->cr3 = memory_page_frame_alloc()) == 0)
		return -1;

	vm_context->init = true;

	memset(vm_context->page_tables_page_frame_refs, 0,
			USER_ADDR_SPACE_PAGE_DIRECTORY_ENTRIES);

	return 0;
}

void vm_context_switch(struct vm_context* vm_context)
{
	paging_switch_cr3(vm_context->cr3, vm_context->init);
	vm_context->init = false;
}
