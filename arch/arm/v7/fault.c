#include <kernel/cpu_context.h>
#include <kernel/mm/vmm.h>
#include <libk/libk.h>
#include "../cpu_context.h"

#include <kernel/log.h>

void data_abort_handler(struct cpu_context* ctx)
{
	v_addr_t fault_addr; // DFAR
	uint32_t dfsr;
	int flags = 0;
	v_addr_t fixup_ret;

	__asm__ ("mrc p15, #0, %0, c6, c0, #0" : "=r" (fault_addr));
	__asm__ ("mrc p15, #0, %0, c5, c0, #0" : "=r" (dfsr));

	log_w_printf("pc=%p, dfsr=%p\n", (void*)fault_addr, (void*)dfsr);

	if (dfsr & (1 << 11))
		flags |= VMM_FAULT_WRITE;
	if (cpu_context_is_usermode(ctx))
		flags |= VMM_FAULT_USER;

	fixup_ret = vmm_handle_page_fault(fault_addr, flags);
	if (fixup_ret)
		ctx->pc = fixup_ret;
}
