#include <arch/paging.h>
#include <kernel/errno.h>
#include <kernel/kmalloc.h>
#include <kernel/memory.h>
#include <kernel/paging.h>
#include <kernel/vm_context.h>
#include <libk/libk.h>

int vm_context_init(struct vm_context* vm_context)
{
	vm_context->cr3 = memory_page_frame_alloc();
	if (!vm_context->cr3)
		return -ENOMEM;

	vm_context->init = true;

	memset(vm_context->page_tables_page_frame_refs, 0,
		   USER_ADDR_SPACE_PAGE_DIRECTORY_ENTRIES);

	return 0;
}

int vm_context_create(struct vm_context** result)
{
	struct vm_context* vm_ctx;
	int err;

	vm_ctx = kmalloc(sizeof(struct vm_context));
	if (!vm_ctx)
		return -ENOMEM;

	err = vm_context_init(vm_ctx);
	if (err) {
		kfree(vm_ctx);
		vm_ctx = NULL;
	}

	*result = vm_ctx;

	return err;
}

void vm_context_reset(struct vm_context* vm_context)
{
	if (vm_context->cr3)
		memory_page_frame_free(vm_context->cr3);
}

void vm_context_destroy(struct vm_context* vm_context)
{
	vm_context_reset(vm_context);
	kfree(vm_context);
}

void vm_context_switch(struct vm_context* vm_context)
{
	paging_switch_cr3(vm_context->cr3, vm_context->init);
	vm_context->init = false;
}
