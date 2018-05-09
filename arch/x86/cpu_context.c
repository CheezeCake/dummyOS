#include <kernel/cpu_context.h>
#include <kernel/kassert.h>
#include <libk/libk.h>
#include <libk/utils.h>
#include "cpu_context.h"
#include "gdt.h"
#include "segment.h"
#include "tss.h"

static void init(struct cpu_context* cpu_context, v_addr_t pc,
				 uint16_t code_segment, uint16_t data_segment)
{
	memset(cpu_context, 0, sizeof(struct cpu_context));

	cpu_context->eip = pc;

	cpu_context->cs = code_segment;
	cpu_context->ds = data_segment;
	cpu_context->es = data_segment;
	cpu_context->fs = data_segment;
	cpu_context->gs = data_segment;
	cpu_context->ss = data_segment;

	cpu_context->eflags = (1 << 9) | (1 << 1); // IF = 1, reserved_1 = 1
}

void cpu_context_kernel_init(struct cpu_context* cpu_context, v_addr_t pc)
{
	init(cpu_context, pc, make_segment_selector(PRIVILEGE_KERNEL, KCODE),
		 make_segment_selector(PRIVILEGE_KERNEL, KDATA));
}

void cpu_context_user_init(struct cpu_context* cpu_context, v_addr_t user_pc,
						   v_addr_t user_stack)
{
	uint16_t user_data_seg = make_segment_selector(PRIVILEGE_USER, UDATA);

	init(cpu_context, user_pc, make_segment_selector(PRIVILEGE_USER, UCODE),
		 user_data_seg);

	// keep ss a kernel segment, it will be set to the user value with user.ss
	cpu_context->ss = make_segment_selector(PRIVILEGE_KERNEL, KDATA);

	cpu_context->user.esp = user_stack;
	cpu_context->user.ss = user_data_seg;
}

bool cpu_context_is_usermode(const struct cpu_context* cpu_context)
{
	return segment_is_user(cpu_context->cs);
}

size_t cpu_context_sizeof(void)
{
	return sizeof(struct cpu_context);
}

void cpu_context_update_tss(const struct cpu_context* cpu_context)
{
	if (cpu_context_is_usermode(cpu_context)) {
		/*
		 * The cpu_context is saved on the thread's kernel stack.
		 * Make the TSS esp0 point just above the currently saved context
		 * that will be "consumed" by cpu_context_switch.
		 */
		tss_update((v_addr_t)cpu_context + sizeof(struct cpu_context));
	}
}


void cpu_context_set_syscall_return_value(struct cpu_context* cpu_context,
										  int ret)
{
	cpu_context->eax = ret;
}

static void cpu_context_set_pc(struct cpu_context* cpu_context, v_addr_t pc)
{
	cpu_context->eip = pc;
}

void cpu_context_set_user_sp(struct cpu_context* cpu_context, v_addr_t sp)
{
	cpu_context->user.esp = sp;
}

v_addr_t cpu_context_get_user_sp(struct cpu_context* cpu_context)
{
	return cpu_context->user.esp;
}

static void cpu_context_pass_user_args(struct cpu_context* cpu_context,
								const v_addr_t* args, size_t n)
{
	v_addr_t* sp = (v_addr_t*)align_down(cpu_context->user.esp,
										 sizeof(v_addr_t));
	// TODO: uaccess copy_to_user
	for (int i = n - 1; i >= 0; --i)
		*(--sp) = args[i];

	cpu_context_set_user_sp(cpu_context, (v_addr_t)sp);
}

void cpu_context_setup_signal_handler(struct cpu_context* cpu_context,
									  v_addr_t handler, v_addr_t sig_trampoline,
									  const v_addr_t* args, size_t n)
{
	cpu_context_pass_user_args(cpu_context, args, n);
	cpu_context_pass_user_args(cpu_context, &sig_trampoline, 1); // ret
	cpu_context_set_pc(cpu_context, handler);
}
