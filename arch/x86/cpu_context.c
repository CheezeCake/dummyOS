#include <kernel/cpu_context.h>
#include <kernel/kassert.h>
#include <kernel/mm/uaccess.h>
#include <libk/libk.h>
#include <libk/utils.h>
#include "cpu_context.h"
#include "gdt.h"
#include "segment.h"
#include "tss.h"

#define CPU_CONTEXT_KERNEL_SIZE offsetof(struct cpu_context, user)

static void cpu_context_init(struct cpu_context* cpu_context, v_addr_t pc,
			     uint16_t code_segment, uint16_t data_segment)
{
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
	memset(cpu_context, 0, CPU_CONTEXT_KERNEL_SIZE);
	cpu_context_init(cpu_context,
			 pc,
			 make_segment_selector(PRIVILEGE_KERNEL, KCODE),
			 make_segment_selector(PRIVILEGE_KERNEL, KDATA));
}

void cpu_context_user_init(struct cpu_context* cpu_context, v_addr_t user_pc,
			   v_addr_t user_stack)
{
	uint16_t user_data_seg = make_segment_selector(PRIVILEGE_USER, UDATA);

	memset(cpu_context, 0, sizeof(struct cpu_context));
	cpu_context_init(cpu_context,
			 user_pc,
			 make_segment_selector(PRIVILEGE_USER, UCODE),
			 user_data_seg);

	// keep ss a kernel segment, it will be set to the user value with user.ss
	cpu_context->ss = make_segment_selector(PRIVILEGE_KERNEL, KDATA);

	cpu_context->user.esp = user_stack;
	cpu_context->user.ss = user_data_seg;
}

	static inline
void cpu_context_kernel_copy(const struct cpu_context* ctx,
			     struct cpu_context* copy)
{
	memcpy(copy, ctx, CPU_CONTEXT_KERNEL_SIZE);
}

	static inline
void cpu_context_user_copy(const struct cpu_context* ctx,
			   struct cpu_context* copy)
{
	memcpy(copy, ctx, sizeof(struct cpu_context));
}

void cpu_context_copy(const struct cpu_context* ctx, struct cpu_context* copy)
{
	cpu_context_is_usermode(ctx)
		? cpu_context_user_copy(ctx, copy)
		: cpu_context_kernel_copy(ctx, copy);
}

bool cpu_context_is_usermode(const struct cpu_context* cpu_context)
{
	return segment_is_user(cpu_context->cs);
}

struct cpu_context* cpu_context_get_previous(struct cpu_context* ctx)
{
	return (cpu_context_is_usermode(ctx))
		? (ctx + 1)
		: (struct cpu_context*)((int8_t*)ctx + CPU_CONTEXT_KERNEL_SIZE);
}

struct cpu_context* cpu_context_get_next_user(struct cpu_context* ctx)
{
	return (ctx - 1);
}

struct cpu_context* cpu_context_get_next_kernel(struct cpu_context* ctx)
{
	return (struct cpu_context*)((int8_t*)ctx - CPU_CONTEXT_KERNEL_SIZE);
}

void cpu_context_update_tss(struct cpu_context* cpu_context)
{
	/*
	 * The cpu_context is saved on the thread's kernel stack.
	 * Make the TSS esp0 point just above the currently saved context
	 * that will be "consumed" by cpu_context_switch.
	 */
	tss_update((v_addr_t)cpu_context_get_previous(cpu_context));
}


void cpu_context_set_syscall_return_value(struct cpu_context* cpu_context,
					  int ret)
{
	cpu_context->eax = ret;
}

void cpu_context_copy_syscall_regs(struct cpu_context* ctx,
				   const struct cpu_context* src)
{
	ctx->eax = src->eax;

	ctx->ebx = src->ebx;
	ctx->ecx = src->ecx;
	ctx->edx = src->edx;
	ctx->esi = src->esi;
	ctx->edi = src->edi;
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

static int cpu_context_pass_user_args(struct cpu_context* cpu_context,
				      const v_addr_t* args, size_t n)
{
	v_addr_t* sp = (v_addr_t*)align_down(cpu_context->user.esp,
					     sizeof(v_addr_t));
	int err;

	sp -= n;

	err = copy_to_user((void*)sp, args, sizeof(args[0]) * n);
	if (!err)
		cpu_context_set_user_sp(cpu_context, (v_addr_t)sp);

	return err;
}

int cpu_context_setup_signal_handler(struct cpu_context* cpu_context,
				     v_addr_t handler, v_addr_t sig_trampoline,
				     const v_addr_t* args, size_t n)
{
	int err;

	err = cpu_context_pass_user_args(cpu_context, args, n);
	if (!err) {
		err = cpu_context_pass_user_args(cpu_context, &sig_trampoline, 1); // ret
		if (!err)
			cpu_context_set_pc(cpu_context, handler);
	}

	return err;
}

int cpu_context_get_syscall_nr(const struct cpu_context* ctx)
{
	return ctx->eax;
}

intptr_t cpu_context_get_syscall_arg_2(const struct cpu_context* ctx)
{
	return ctx->ecx;
}
