#include <kernel/sched.h>
#include <kernel/cpu_context.h>
#include <kernel/time.h>
#include <kernel/kassert.h>
#include <kernel/libk.h>

#include <kernel/log.h>

static unsigned int quantum;

static struct thread* current_thread;
static struct thread_list ready_list;
static struct time current_thread_start;

void sched_init(unsigned int quantum_in_ms)
{
	quantum = quantum_in_ms;

	list_init_null(&ready_list);

	current_thread = NULL;
	current_thread_start.sec = 0;
	current_thread_start.nano_sec = 0;
}

static void sched_switch_to_next_thread(void)
{
	struct thread* previous = current_thread;
	if (previous)
		list_push_back(&ready_list, previous);

	current_thread = list_front(&ready_list);
	list_pop_front(&ready_list);
	current_thread_start = time_get_current();

	log_printf("switching from %s to %s\n", previous->name, current_thread->name);
	struct cpu_context *ctx = (previous) ? previous->cpu_context : NULL;
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
	list_push_back(&ready_list, thread);
}

// XXX: protect with irq_disable?
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

	const struct time current_time = time_get_current();
	if (time_diff_ms(&current_time, &current_thread_start) > quantum)
		sched_switch_to_next_thread();
}
