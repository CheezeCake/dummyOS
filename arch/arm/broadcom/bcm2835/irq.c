#include <arch/broadcom/bcm2835/irq.h>
#include <arch/broadcom/bcm2835/peripherals.h>
#include <arch/irq.h>
#include <dummyos/errno.h>
#include <kernel/types.h>
#include <libk/bits.h>
#include <libk/libk.h>


#define ARM_INTC_REG (ARM_INT_BASE + 0x200)

#define IRQ_NR 72

struct bcm2835_intr_reg
{
	uint32_t irq_basic_pending;
	uint32_t irq_gpu_pending1;
	uint32_t irq_gpu_pending2;
	uint32_t fiq_control;
	uint32_t irq_gpu_enable1;
	uint32_t irq_gpu_enable2;
	uint32_t irq_basic_enable;
	uint32_t irq_gpu_disable1;
	uint32_t irq_gpu_disable2;
	uint32_t irq_basic_disable;
};

static volatile struct bcm2835_intr_reg* const intr_reg = (volatile struct bcm2835_intr_reg*)ARM_INTC_REG;

static irq_handler_t irq_handlers[IRQ_NR];

void bcm2835_irq_init(void)
{
	intr_reg->irq_basic_disable = ~0;
	intr_reg->irq_gpu_disable1 = ~0;
	intr_reg->irq_gpu_disable2 = ~0;
	memset(irq_handlers, 0, sizeof(irq_handlers));
}

static inline void enable_irq(volatile uint32_t* enable_reg, unsigned irq)
{
	*enable_reg |= BIT(irq);
}

#define irq_is_basic(irq)	((irq) >= 64)
#define irq_is_gpu2(irq)	((irq) >= 32 && (irq) < 64)
#define irq_is_gpu1(irq)	((irq) < 32)

int bcm2835_set_irq_handler(enum bcm2835_irq irq, irq_handler_t handler)
{
	if (irq >= IRQ_NR)
		return -EINVAL;

	irq_handlers[irq] = handler;

	if (irq_is_basic(irq))
		enable_irq(&intr_reg->irq_basic_enable, irq - 64);
	else if (irq_is_gpu2(irq))
		enable_irq(&intr_reg->irq_gpu_enable2, irq - 32);
	else
		enable_irq(&intr_reg->irq_gpu_enable1, irq);

	return 0;
}

static inline bool is_pending(volatile uint32_t* pending_reg, unsigned irq)
{
	return (*pending_reg & BIT(irq));
}

static bool irq_is_pending(unsigned irq)
{
	if (irq_is_basic(irq))
		return is_pending(&intr_reg->irq_basic_pending, irq - 64);
	else if (irq_is_gpu2(irq))
		return is_pending(&intr_reg->irq_gpu_pending2, irq - 32);
	else
		return is_pending(&intr_reg->irq_gpu_pending1, irq);
}


void bcm2835_irq_handler(void)
{
	for (unsigned irq = 0; irq < IRQ_NR; ++irq) {
		if (irq_is_pending(irq) && irq_handlers[irq]) {
			irq_handlers[irq]();
			return;
		}
	}
}
