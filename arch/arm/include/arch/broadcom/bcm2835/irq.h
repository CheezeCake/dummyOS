#ifndef _ARCH_BROADCOM_BCM2835_IRQ_H_
#define _ARCH_BROADCOM_BCM2835_IRQ_H_

enum bcm2835_irq
{
	IRQ_SYSTEM_TIMER_0	= 0, // do not enable, already used by the GPU
	IRQ_SYSTEM_TIMER_1	= 1,
	IRQ_SYSTEM_TIMER_2	= 2,
	IRQ_SYSTEM_TIMER_3	= 3,

	IRQ_USB_CONTROLLER	= 9,

	IRQ_AUX			= 29,

	IRQ_PCM_AUDIO		= 55,

	IRQ_UART		= 57,

	IRQ_SD_HOST_CONTROLLER	= 62,

	IRQ_ARM_TIMER		= 64,
};

void bcm2835_irq_init(void);
void bcm2835_irq_handler(void);

typedef void (*irq_handler_t)(void);
int bcm2835_set_irq_handler(enum bcm2835_irq irq, irq_handler_t handler);

#endif
