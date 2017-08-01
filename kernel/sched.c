#include <kernel/sched.h>
#include <kernel/cpu_context.h>
#include <kernel/time.h>
#include <kernel/kassert.h>
#include <kernel/libk.h>

#include <kernel/log.h>

static unsigned int quantum;

static struct thread* current_thread = NULL;
static struct thread_list ready_list;
static struct time current_thread_start = { .sec = 0, .nano_sec = 0 };

void sched_init(unsigned int quantum_in_ms)
{
	quantum = quantum_in_ms;

	list_init_null(&ready_list);
}

static void sched_switch_to_next_thread(void)
{
	if (list_empty(&ready_list))
		return;

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
	kassert(current_thread == NULL);
	kassert(!list_empty(&ready_list));

	sched_switch_to_next_thread();
}

void sched_add_thread(struct thread* thread)
{
	log_printf("sched add %s\n", thread->name);
	thread->state = THREAD_READY;
	list_push_back(&ready_list, thread);
}

struct thread* sched_get_current_thread(void)
{
	return current_thread;
}

void sched_block_current_thread(void)
{
	log_printf("sched blocking %s\n", current_thread->name);
	current_thread->state = THREAD_BLOCKED;
	sched_switch_to_next_thread();
}

void sched_remove_current_thread(void)
{
	if (!current_thread)
		return;

	thread_destroy(current_thread);
	current_thread = NULL;

	sched_switch_to_next_thread();
}

void sched_schedule(void)
{
	if (list_empty(&ready_list))
		return;

	struct time current_time;
	time_get_current(&current_time);
	if (time_diff_ms(&current_time, &current_thread_start) > quantum) {
		if (current_thread) {
			current_thread->state = THREAD_READY;
			list_push_back(&ready_list, current_thread);
		}

		sched_switch_to_next_thread();
	}
}

void sched_yield_current_thread(void)
{
	if (!list_empty(&ready_list) && current_thread) {
		current_thread->state = THREAD_READY;
		list_push_back(&ready_list, current_thread);

		sched_switch_to_next_thread();
	}
}

void sched_sleep_current_thread(unsigned int millis)
{
	current_thread->state = THREAD_BLOCKED;
	time_get_current(&current_thread->waiting_for.until);
	time_add_millis(&current_thread->waiting_for.until, millis);

	time_add_waiting_thread(current_thread);
	sched_switch_to_next_thread();
}
