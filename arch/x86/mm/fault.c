#include <kernel/cpu_context.h>
#include <kernel/mm/vmm.h>
#include <libk/libk.h>
#include "../cpu_context.h"

#include <kernel/log.h>

struct pf_error_code
{
	uint8_t present:1;
	uint8_t write:1;
	uint8_t user:1;
	uint32_t reserved:29;
} __attribute__((packed));
static_assert(sizeof(struct pf_error_code) == 4, "sizeof(pf_error_code)");

void handle_page_fault(int exception, struct cpu_context* ctx)
{
	struct pf_error_code error;
	v_addr_t fault_addr;
	int flags = 0;
	v_addr_t fixup_ret;

	__asm__ volatile ("movl %%cr2, %0" : "=r" (fault_addr));
	memcpy(&error, &ctx->error_code, sizeof(struct pf_error_code));

	log_e_printf("eip=%p, code=%p\n", (void*)ctx->eip, (void*)ctx->error_code);
	if (!error.present)
		flags |= VMM_FAULT_NOT_PRESENT;
	if (error.write)
		flags |= VMM_FAULT_WRITE;
	if (error.user)
		flags |= VMM_FAULT_USER;

	fixup_ret = vmm_handle_page_fault(fault_addr, flags);
	if (fixup_ret)
		ctx->eip = fixup_ret;
}
