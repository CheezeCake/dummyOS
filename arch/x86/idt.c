#include <stddef.h>
#include <stdint.h>
#include <kernel/interrupt.h>
#include "idt.h"
#include "segment.h"

// defined in interrupt.S
extern uint32_t asm_interrupt_handlers[INTERRUPT_MAX];

static struct idt_gate_descriptor* idt = IDT_ADDRESS;

int idt_set_handler(uint8_t index, enum gate_type type)
{
	uint32_t handler = asm_interrupt_handlers[index];
	if (handler == 0)
		return -1;

	idt[index].offset_15_0 = handler & 0xffff;
	idt[index].offset_31_16 = (handler >> 16) & 0xffff;
	idt[index].type = type;
	idt[index].dpl = (type == INTGATE) ? 0 : 3;
	idt[index].present = 1;

	return 0;
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
		idt[i].segment_selector = make_segment_register(0, false, KCODE);
		idt[i].reserved = 0;
		idt[i].flags = 0;
		idt[i].gate_size = 1;
		idt[i].zero = 0;

		idt_unset_handler(i);
	}

	idt_register.base_address = (uint32_t)idt;
	idt_register.limit = (IDT_SIZE * sizeof(struct idt_gate_descriptor)) - 1;

	// load the idt register
	__asm__ __volatile__ (
			"lidt %0"
			:
			: "m" (idt_register)
			: "memory");

}
