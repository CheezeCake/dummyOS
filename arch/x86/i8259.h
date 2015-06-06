#ifndef _I8259_H_
#define _I8259_H_

#define PIC_MASTER 0x20
#define PIC_SLAVE 0xa0
#define PIC_MASTER_DATA (PIC_MASTER + 1)
#define PIC_SLAVE_DATA (PIC_SLAVE + 1)

#define PIC_SLAVE_MIN_LINE 8

void i8259_init(void);
void i8259_irq_enable(uint8_t irq_line);
void i8259_irq_disable(uint8_t irq_line);

#endif
