#include <arch/cpu.h>
#include <dummyos/compiler.h>
#include <kernel/kassert.h>
#include <kernel/time/time.h>

#include "peripherals.h"
#include "timer.h"

enum timer_registers
{
	TIMER_INT_ROUTING	= 0x24 >> 2,

	TIMER_CONTROL_STATUS	= 0x34 >> 2,
	TIMER_CLEAR_RELOAD	= 0x38 >> 2,
};

struct timer_int_routing_reg
{
	enum {
		CORE0_IRQ = 0,
		CORE1_IRQ = 1,
		CORE2_IRQ = 2,
		CORE3_IRQ = 3,
		CORE0_FIQ = 4,
		CORE1_FIQ = 5,
		CORE2_FIQ = 6,
		CORE3_FIQ = 7,
	} route:3;
	uint32_t unused3_31:29;
};
static_assert_type_size(struct timer_int_routing_reg, 4);

struct timer_control_status_reg
{
	uint32_t reload_value:28;
	uint32_t timer_enable:1;
	uint32_t int_enable:1;
	uint32_t unused30:1;
	uint32_t int_flag:1;
};
static_assert_type_size(struct timer_control_status_reg, 4);

struct timer_clear_reload
{
	uint32_t unused:30;
	uint32_t reload:1;
	uint32_t clear_int:1;
};
static_assert_type_size(struct timer_clear_reload, 4);


volatile uint32_t* const timer_regs = (volatile uint32_t*)BCM2836_TIMER_BASE;

static inline volatile struct timer_control_status_reg* get_control_status(void)
{
	return (struct timer_control_status_reg*)&timer_regs[TIMER_CONTROL_STATUS];
}

static inline volatile struct timer_int_routing_reg* get_timer_int_routing(void)
{
	return (volatile struct timer_int_routing_reg*)&timer_regs[TIMER_INT_ROUTING];
}

static inline volatile struct timer_clear_reload* get_timer_clear_reload(void)
{
	return (struct timer_clear_reload*)&timer_regs[TIMER_CLEAR_RELOAD];
}

bool bcm2836_timer_irq(void)
{
	volatile struct timer_control_status_reg* timer_ctrl_status =
		get_control_status();
	if (!timer_ctrl_status->int_flag)
		return false;
	time_tick();
	volatile struct timer_clear_reload* timer_clear_reload =
		get_timer_clear_reload();
	timer_clear_reload->clear_int = 1;
	timer_clear_reload->reload = 1;

	return true;
}

int bcm2836_timer_init(uint32_t usec)
{
	const int core_irq[4] = {
		CORE0_IRQ,
		CORE1_IRQ,
		CORE2_IRQ,
		CORE3_IRQ,
	};
	const unsigned core_id = cpu_get_core_id();
	kassert(core_id < sizeof(core_irq));

	volatile struct timer_int_routing_reg* timer_int_routing =
		get_timer_int_routing();
	timer_int_routing->route = core_irq[core_id];

	volatile struct timer_control_status_reg* timer_ctrl_status =
		get_control_status();
	// timer frequency: 38.4 Mhz
	timer_ctrl_status->reload_value = usec * 384 / 10;
	timer_ctrl_status->timer_enable = 1;
	timer_ctrl_status->int_enable = 1;

	volatile struct timer_clear_reload* timer_clear_reload =
		get_timer_clear_reload();
	timer_clear_reload->clear_int = 1;
	timer_clear_reload->reload = 1;

	return 0;
}
