#ifndef _KERNEL_TIMER_H_
#define _KERNEL_TIMER_H_

#include <kernel/time/time.h>
#include <kernel/kmalloc.h>
#include <libk/list.h>
#include <libk/refcount.h>

typedef void (*timer_callback_t)(struct timer*);

struct timer
{
	struct time time;
	timer_callback_t cb;

	list_node_t t_list; // time.c::timer_list node
};


/**
 * @brief Initializes a struct timer
 *
 * @param timer the timer
 * @param delay_ms the delay in ms
 * @param cb the callback called when the delay has expired
 * @return 0 on success
 */
int timer_init(struct timer* timer, unsigned delay_ms, timer_callback_t cb);

/**
 * @brief resets a struct timer
 */
void timer_reset(struct timer* timer)

/**
 * Registers a timer to the time subsystem
 */
void timer_register(struct timer* timer);


/**
 * @brief Calls the callback function of a timer
 */
void timer_trigger(struct timer* timer);

#endif
