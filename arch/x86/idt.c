#include <kernel/kassert.h>
#include "idt.h"
#include "segment.h"

// defined in interrupt.S
extern uint32_t asm_interrupt_handlers[INTERRUPTS_DEFINED];

static struct idt_gate_descriptor* const idt = IDT_ADDRESS;


static int set_handler(uint8_t index, enum gate_type type, uint32_t handler)
{
	if (handler == 0 || idt[index].present)
		return -1;

	idt[index].offset_15_0 = handler & 0xffff;
	idt[index].offset_31_16 = handler >> 16;
	idt[index].type = type;
	idt[index].dpl = (type == INTGATE) ? PRIVILEGE_KERNEL : PRIVILEGE_USER;
	idt[index].present = 1;

	return 0;
}

int idt_set_direct_handler(uint8_t int_number, enum gate_type type, interrupt_handler_t handler)
{
	kassert(int_number >= INTERRUPTS_DEFINED); // do not use a irq/exception number
	return set_handler(int_number, type, (uint32_t)handler);
}

int idt_set_handler(uint8_t index, enum gate_type type)
{
	return set_handler(index, type, asm_interrupt_handlers[index]);
}

void idt_unset_handler(uint8_t index)
{
	idt[index].offset_15_0 = 0;
	idt[index].offset_31_16 = 0;
	idt[index].type = INTGATE;
	idt[index].dpl = 0;
	idt[index].present = 0; // no handler
}

void idt_init(void)
{
	struct idtr idt_register;

	for (int i = 0; i < IDT_SIZE; i++) {
		idt[i].segment_selector =
			make_segment_selector(PRIVILEGE_KERNEL, KCODE);
		idt[i].reserved = 0;
		idt[i].flags = 0;
		idt[i].gate_size = 1;
		idt[i].zero = 0;

		idt_unset_handler(i);
	}

	idt_register.base_address = (uint32_t)idt;
	idt_register.limit = IDT_SIZE_BYTES;

	// load the idt register
	__asm__ __volatile__ (
			"lidt %0"
			:
			: "m" (idt_register)
			: "memory");

}
