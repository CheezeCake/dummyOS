#include <kernel/sched.h>
#include <kernel/cpu_context.h>
#include <kernel/time/time.h>
#include <kernel/time/timer.h>
#include <kernel/kassert.h>
#include <kernel/libk.h>
#include <kernel/thread_list.h>
#include <arch/irq.h>

#include <kernel/log.h>

static unsigned int quantum;

// current thread
static struct thread* current_thread = NULL;
static struct time current_thread_start = { .sec = 0, .nano_sec = 0 };

// ready queue & ready buffer
static struct thread_list ready_list;
static struct thread_list_synced ready_buffer;

static inline void copy_buffer_to_ready_queue()
{
	if (list_locked(&ready_buffer))
		return;

	struct thread* it;

	list_lock_synced(&ready_buffer);

	it = list_begin(&ready_buffer);
	while (it) {
		struct thread* tmp = it;
		it = list_it_next(it);

		list_push_back(&ready_list, tmp); // add to ready queue
	}

	list_clear(&ready_buffer); // clear ready buffer

	list_unlock_synced(&ready_buffer);
}

void sched_init(unsigned int quantum_in_ms)
{
	quantum = quantum_in_ms;

	list_init_null(&ready_list);
	list_init_null_synced(&ready_buffer);
}

static void sched_preempt(void)
{
	copy_buffer_to_ready_queue();

	kassert(!list_empty(&ready_list));

	log_printf("switching from %s ", (current_thread) ? current_thread->name : NULL);

	struct cpu_context* ctx = (current_thread) ? current_thread->cpu_context : NULL;

	current_thread = list_front(&ready_list);
	list_pop_front(&ready_list);

	current_thread->state = THREAD_RUNNING;
	time_get_current(&current_thread_start);

	log_printf("to %s\n", current_thread->name);
	cpu_context_switch(ctx, current_thread->cpu_context);
}

void sched_start(void)
{
	log_puts("sched_start()\n");

	kassert(current_thread == NULL);

	irq_enable();
	sched_preempt();
}

void sched_add_thread(struct thread* thread)
{
	kassert(thread != NULL);

	log_printf("sched add %s\n", thread->name);
	thread->state = THREAD_READY;

	// add to synced buffer
	list_push_back_synced(&ready_buffer, thread);
}

struct thread* sched_get_current_thread(void)
{
	return current_thread;
}

void sched_block_current_thread(void)
{
	log_printf("sched blocking %s\n", current_thread->name);
	current_thread->state = THREAD_BLOCKED;
	sched_preempt();
}

void sched_remove_current_thread(void)
{
	if (!current_thread)
		return;

	thread_destroy(current_thread);
	current_thread = NULL;

	sched_preempt();
}

/*
 * called from time tick interrupt handler
 */
void sched_schedule(void)
{
	copy_buffer_to_ready_queue();

	if (list_empty(&ready_list))
		return;

	struct time current_time;
	time_get_current(&current_time);
	if (time_diff_ms(&current_time, &current_thread_start) > quantum) {
		if (current_thread) {
			current_thread->state = THREAD_READY;
			list_push_back(&ready_list, current_thread);
		}

		sched_preempt();
	}
}

void sched_yield_current_thread(void)
{
	if (list_empty(&ready_list))
		return;

	current_thread->state = THREAD_READY;
	list_push_back_synced(&ready_buffer, current_thread);

	irq_disable();
	sched_preempt();
	irq_enable();
}

static void sched_timer_callback(void* data)
{
	struct thread* thread = (struct thread*)data;
	sched_add_thread(thread);
}

void sched_sleep_current_thread(unsigned int millis)
{
	current_thread->state = THREAD_BLOCKED;

	struct timer* timer = timer_create(millis, sched_timer_callback, current_thread);
	kassert(timer != NULL);
	time_add_timer(timer);

	irq_disable();
	sched_preempt();
	irq_enable();
}
