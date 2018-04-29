#include <kernel/errno.h>
#include <kernel/kassert.h>
#include "exception.h"
#include "idt.h"
#include "irq.h"

/*
 * 8 byte gate descriptor structure
 */
struct idt_gate_descriptor
{
	uint16_t offset_15_0;

	uint16_t segment_selector;

	uint8_t reserved:5;
	uint8_t flags:3;
	uint8_t type:3;
	uint8_t gate_size:1;
	uint8_t zero:1;
	uint8_t dpl:2;
	uint8_t present:1;

	uint16_t offset_31_16;
} __attribute__ ((packed));

/*
 * IDT register
 */
struct idtr
{
	uint16_t limit;
	uint32_t base_address;
} __attribute__ ((packed));

// defined in irq.S
extern uint32_t asm_irq_handlers[IRQ_COUNT];
// defined in exception.S
extern uint32_t asm_exception_handlers[EXCEPTION_COUNT];
// defined in syscall.S
void syscall_handler(void);

static struct idt_gate_descriptor idt[IDT_SIZE];


static int set_handler_present(uint8_t index, enum gate_type type,
							   enum privilege_level dpl, v_addr_t handler)
{
	if (!handler)
		return -EINVAL;

	idt[index].offset_15_0 = handler & 0xffff;
	idt[index].offset_31_16 = handler >> 16;
	idt[index].type = type;
	idt[index].dpl = dpl;
	idt[index].present = 1;

	return 0;
}

int idt_set_syscall_handler(uint8_t int_number)
{
	// do not use a irq/exception number
	kassert(int_number > INTERRUPTS_DEFINED);

	return set_handler_present(int_number, TRAPGATE, PRIVILEGE_USER,
							   (v_addr_t)syscall_handler);
}


int idt_set_exception_handler_present(uint8_t exception, enum gate_type type)
{
	return set_handler_present(EXCEPTION_IDT_INDEX(exception), type,
							   PRIVILEGE_KERNEL,
							   asm_exception_handlers[exception]);
}

int idt_set_irq_handler_present(uint8_t irq, enum gate_type type)
{
	return set_handler_present(IRQ_IDT_INDEX(irq), type, PRIVILEGE_KERNEL,
							   asm_irq_handlers[irq]);
}

static void unset_handler_present(uint8_t index)
{
	idt[index].offset_15_0 = 0;
	idt[index].offset_31_16 = 0;
	idt[index].type = INTGATE;
	idt[index].dpl = 0;
	idt[index].present = 0; // no handler
}

void idt_unset_exception_handler_present(uint8_t exception)
{
	unset_handler_present(EXCEPTION_IDT_INDEX(exception));
}

void idt_unset_irq_handler_present(uint8_t irq)
{
	unset_handler_present(IRQ_IDT_INDEX(irq));
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

		unset_handler_present(i);
	}

	idt_register.base_address = (uint32_t)idt;
	idt_register.limit = sizeof(idt);

	// load the idt register
	__asm__ volatile ("lidt %0" : : "m" (idt_register) : "memory");
}
