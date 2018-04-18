#include <dummyos/compiler.h>
#include <kernel/cpu_context.h>
#include <kernel/kassert.h>
#include <libk/libk.h>
#include "gdt.h"
#include "segment.h"
#include "tss.h"

struct cpu_context
{
	uint32_t eax;
	uint32_t ebx;
	uint32_t ecx;
	uint32_t edx;

	uint32_t esi;
	uint32_t edi;
	uint32_t ebp;

	uint16_t ds;
	uint16_t es;
	uint16_t fs;
	uint16_t gs;
	uint16_t ss;
	uint16_t alignment;

	// pushed by the CPU
	uint32_t error_code;
	uint32_t eip;
	uint32_t cs;
	uint32_t eflags;
	struct
	{
		uint32_t esp;
		uint32_t ss;
	} user;
} __attribute__((packed));
static_assert(sizeof(struct cpu_context) == 64, "sizeof cpu_context != 64");

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
