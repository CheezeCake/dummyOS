#include <kernel/kassert.h>
#include <kernel/mm/uaccess.h>
#include <libk/libk.h>
#include <libk/utils.h>

#include "cpu_context.h"
#include "exception.h"

static void cpu_context_init(struct cpu_context* cpu_context, v_addr_t pc,
			     uint32_t cpsr)
{
	memset(cpu_context, 0, sizeof(struct cpu_context));

	cpu_context->pc = pc;
	cpu_context->cpsr = cpsr;
}

void cpu_context_kernel_init(struct cpu_context* cpu_context, v_addr_t pc)
{
	cpu_context_init(cpu_context, pc, ARM_MODE_SVC);
}

void cpu_context_user_init(struct cpu_context* cpu_context, v_addr_t user_pc,
			   v_addr_t user_stack)
{
	cpu_context_init(cpu_context, user_pc, ARM_MODE_USR);
	cpu_context->sp = user_stack;
}

void cpu_context_copy(const struct cpu_context* ctx, struct cpu_context* copy)
{
	memcpy(copy, ctx, sizeof(struct cpu_context));
}

bool cpu_context_is_usermode(const struct cpu_context* cpu_context)
{
	return !(cpu_context->cpsr & 0xf);
}

struct cpu_context* cpu_context_get_previous(struct cpu_context* ctx)
{
	return (ctx + 1);
}

static inline struct cpu_context* cpu_context_get_next(struct cpu_context* ctx)
{
	return (ctx - 1);
}

struct cpu_context* cpu_context_get_next_user(struct cpu_context* ctx)
{
	return cpu_context_get_next(ctx);
}

struct cpu_context* cpu_context_get_next_kernel(struct cpu_context* ctx)
{
	return cpu_context_get_next(ctx);
}

void cpu_context_set_syscall_return_value(struct cpu_context* cpu_context,
					  int ret)
{
	cpu_context->r0 = ret;
}

void cpu_context_copy_syscall_regs(struct cpu_context* ctx,
				   const struct cpu_context* src)
{
	ctx->r0 = src->r0;

	ctx->r1 = src->r1;
	ctx->r2 = src->r2;
	ctx->r3 = src->r3;
	ctx->r4 = src->r4;
	ctx->r5 = src->r5;
}

static void cpu_context_set_pc(struct cpu_context* cpu_context, v_addr_t pc)
{
	cpu_context->pc = pc;
}

void cpu_context_set_user_sp(struct cpu_context* cpu_context, v_addr_t sp)
{
	cpu_context->sp = sp;
}

v_addr_t cpu_context_get_user_sp(struct cpu_context* cpu_context)
{
	return cpu_context->sp;
}

int cpu_context_setup_signal_handler(struct cpu_context* cpu_context,
				     v_addr_t handler, v_addr_t sig_trampoline,
				     const v_addr_t* args, size_t n)
{
	uint32_t* const arg_reg[] = {
		&cpu_context->r0,
		&cpu_context->r1,
		&cpu_context->r2,
		&cpu_context->r3,
	};

	kassert(n < ARRAY_SIZE(arg_reg));

	for (size_t i = 0; i < n; ++i)
		*arg_reg[i] = args[i];

	cpu_context->lr = sig_trampoline;
	cpu_context_set_pc(cpu_context, handler);

	return 0;
}

int cpu_context_get_syscall_nr(const struct cpu_context* ctx)
{
	return ctx->r0;
}

intptr_t cpu_context_get_syscall_arg_2(const struct cpu_context* ctx)
{
	return ctx->r2;
}
