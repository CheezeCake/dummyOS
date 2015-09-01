#ifndef _IDT_H_
#define _IDT_H_

// place the IDT at adress 0 in physical memory
#define IDT_ADDRESS 0
#define IDT_SIZE 256

#define INTGATE 0x6 // 110b
#define TRAPGATE 0x7 // 111b

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

int idt_set_handler(unsigned int index, uint8_t type);
int idt_unset_handler(unsigned int index);
void idt_init(void);

#endif
