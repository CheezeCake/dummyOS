#ifndef _IDT_H_
#define _IDT_H_

#include <kernel/interrupt.h>
#include <kernel/types.h>
#include "segment.h"

#define INTERRUPT_MAX 256
// asm_interrupt_hanlders size in interrupt.S
#define INTERRUPTS_DEFINED 48

#define IDT_SIZE INTERRUPT_MAX

enum gate_type
{
	TASKGATE = 0x5, // 101b [unused]
	INTGATE = 0x6, // 110b
	TRAPGATE = 0x7 // 111b
};

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

int idt_set_handler(uint8_t index, enum gate_type type);
int idt_set_syscall_handler(uint8_t int_number, interrupt_handler_t handler);
void idt_unset_handler(uint8_t index);
void idt_init(void);

#endif
