#include <arch/paging.h>
#include <kernel/kassert.h>
#include <kernel/mm/vmm.h>
#include <kernel/sched/sched.h>
#include <kernel/signal.h>
#include <kernel/terminal.h>

#include "exception.h"

void reset_handler(struct cpu_context* ctx)
{
	terminal_puts("RESET HANDLER\n");
	while (1) { }
}
void prefetch_abort_handler(struct cpu_context* ctx)
{
	process_kill(sched_get_current_process()->pid, SIGSEGV);
	sched_yield();
}

void undefined_instruction_handler(struct cpu_context* ctx)
{
	process_kill(sched_get_current_process()->pid, SIGILL);
	sched_yield();
}

void fast_irq_handler(void)
{
	terminal_puts("FIQ HANDLER\n");
}

void exception_init(void)
{
	static _Alignas(8) char irq_stack[1024 + 1];
	extern char exception_vectors;

	kassert(page_is_aligned((v_addr_t)&exception_vectors));
	vmm_map_kernel_page(paging_virt_to_phys((v_addr_t)&exception_vectors),
			  0xffff0000, VMM_PROT_EXEC);

	uint32_t sctlr;
	__asm__ volatile ("mrc p15, #0, %0, c1, c0, #0" : "=r" (sctlr));
	sctlr |= (1 << 13); // enable high exception vectors
	__asm__ volatile ("mcr p15, #0, %0, c1, c0, #0" : : "r" (sctlr));

	exception_set_mode_stack(ARM_MODE_IRQ, irq_stack + sizeof(irq_stack) - 1);
}
