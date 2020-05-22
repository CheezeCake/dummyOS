#include <arch/broadcom/bcm2835/irq.h>
#include <arch/broadcom/bcm2835/peripherals.h>
#include <kernel/time/time.h>

#include "timer.h"

#define SYSTEM_TIMER_BASE (PERIPHERALS_BASE + 0x3000)

struct system_timer_regs
{
	struct {
		uint8_t timer_match0:1;
		uint8_t timer_match1:1;
		uint8_t timer_match2:1;
		uint8_t timer_match3:1;
		uint32_t reserved: 28;
	} control;
	uint32_t low;
	uint32_t high;
	uint32_t compare[4];
};

static volatile struct system_timer_regs* const system_timer_regs = (volatile struct system_timer_regs*)SYSTEM_TIMER_BASE;


static uint32_t usec_tick_interval;

static inline void system_timer_set_next_tick(void)
{
	system_timer_regs->compare[1] = system_timer_regs->low + usec_tick_interval;
}

static void system_timer_tick(void)
{
	system_timer_regs->control.timer_match1 = 1;
	system_timer_set_next_tick();
	time_tick();
}

static int system_timer_init(uint32_t usec)
{
	int err;

	err = bcm2835_set_irq_handler(IRQ_SYSTEM_TIMER_1, system_timer_tick);
	if (err)
		return err;

	usec_tick_interval = usec;

	system_timer_set_next_tick();

	return 0;
}

#if 0
#define ARM_TIMER_BASE (ARM_INT_BASE + 0x400)

struct arm_timer_regs
{
	uint32_t load;
	uint32_t value;
	struct {
		uint32_t unused0:1;
		enum {
			CNTR_16BIT = 0,
			CNTR_23BIT = 1, // 23, not 32?? Typo in bcm2835 docs?
		} cntr:1;
		enum {
			PRESCALE_1 = 0,
			PRESCALE_16 = 1,
			PRESCALE_256 = 2,
			// 3, undefined
		} prescale:2;
		uint32_t unused4:1;
		uint32_t int_enable:1;
		uint32_t unused6:1;
		uint32_t enable:1;
		uint32_t debug_halt:1;
		uint32_t enable_free_running:1;
		uint32_t unused10_15:6;
		uint8_t free_running_cntr_prescaler;
		uint8_t unused24_31;
	} control;
	uint32_t irq_clear;
	uint32_t raw_riq;
	uint32_t masked_riq;
	uint32_t reload;
	uint32_t pre_divider;
	uint32_t free_running_cntr;
};

static volatile struct arm_timer_regs* const arm_timer_regs = (volatile struct arm_timer_regs*)ARM_TIMER_BASE;

static void arm_timer_tick(void)
{
}

static int arm_timer_init(uint32_t usec)
{
	int err;

	err = bcm2835_set_irq_handler(IRQ_ARM_TIMER, arm_timer_tick);
	if (err)
		return err;

	arm_timer_regs->load = 0x400;

	arm_timer_regs->control.cntr = CNTR_23BIT;
	arm_timer_regs->control.enable = 1;
	arm_timer_regs->control.int_enable = 1;
	arm_timer_regs->control.prescale = PRESCALE_256;

	return 0;
}
#endif

int bcm2835_timer_init(uint32_t usec)
{
	return system_timer_init(usec);
}
