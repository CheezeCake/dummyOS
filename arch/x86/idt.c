#include <stddef.h>
#include <stdint.h>
#include "idt.h"
#include "segment.h"
#include "interrupt.h"

// defined in interrupt.S
extern uint32_t asm_interrupt_handlers[INTERRUPT_MAX];

static struct idt_gate_descriptor idt[IDT_SIZE];

int idt_set_handler(unsigned int index, uint8_t type)
{
	if (index < 0 || index > IDT_SIZE ||
			(type != INTGATE && type != TRAPGATE))
		return -1;


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

int idt_unset_handler(unsigned int index)
{
	if (index < 0 || index > IDT_SIZE)
		return -1;

	idt[index].offset_15_0 = 0;
	idt[index].offset_31_16 = 0;
	idt[index].type = INTGATE;
	idt[index].dpl = 0;
	idt[index].present = 0; // no handler

	return 0;
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

		idt_set_handler(i, INTGATE);
	}

	idt_register.base_address = (uint32_t)idt;
	idt_register.limit = sizeof(idt) - 1;

	// load the idt register
	__asm__ __volatile__ (
			"lidt %0"
			:
			: "m" (idt_register)
			: "memory");

}
