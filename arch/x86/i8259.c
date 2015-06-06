#include "io_ports.h"
#include "i8259.h"

void i8259_init(void)
{
	// ICW1: start initialization
	outb(PIC_MASTER, 0x11);
	outb(PIC_SLAVE, 0x11);

	// ICW2: vector offsets
	outb(PIC_MASTER_DATA, 0x20);
	outb(PIC_SLAVE_DATA, 0x28);

	// ICW3
	outb(PIC_MASTER_DATA, 4); // slave PIC in cascade on IRQ2 (0000 0100b = 4)
	outb(PIC_SLAVE_DATA, 2); // slave cascade identity (0000 0010 = 2)

	// ICW4
	outb(PIC_MASTER_DATA, 1);
	outb(PIC_SLAVE_DATA, 1);

	// mask all IRQs (bit set = disabled)
	outb(PIC_MASTER_DATA, 0xfb); // cascade enabled (1011b = 0xb)
	outb(PIC_SLAVE_DATA, 0xff);
}

void i8259_irq_enable(uint8_t irq_line)
{
	uint16_t port;

	if (irq_line < PIC_SLAVE_MIN_LINE) {
		port = PIC_MASTER_DATA;
	}
	else {
		port = PIC_SLAVE_DATA;
		irq_line -= PIC_SLAVE_MIN_LINE;
	}

	outb(port, inb(port) & ~(1 << irq_line));
}

void i8259_irq_disable(uint8_t irq_line)
{
	uint16_t port;

	if (irq_line < PIC_SLAVE_MIN_LINE) {
		port = PIC_MASTER_DATA;
	}
	else {
		port = PIC_SLAVE_DATA;
		irq_line -= PIC_SLAVE_MIN_LINE;
	}

	outb(port, inb(port) | (1 << irq_line));
}
