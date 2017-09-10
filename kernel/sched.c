#include <kernel/sched.h>
#include <kernel/context_switch.h>
#include <kernel/time/time.h>
#include <kernel/time/timer.h>
#include <kernel/kassert.h>
#include <kernel/libk.h>
#include <arch/irq.h>

#include <kernel/log.h>

static unsigned int quantum;

// current thread
static struct thread_list_node* current_thread_node = NULL;
static struct time current_thread_start = { .sec = 0, .nano_sec = 0 };

static struct thread_list ready_queue;

static void sched_preempt(void)
{
	kassert(!list_empty(&ready_queue));

	struct thread* from = (current_thread_node) ? current_thread_node->thread : NULL;
	log_printf("switching from %s ", (from) ? from->name : NULL);

	current_thread_node = list_front(&ready_queue);
	list_pop_front(&ready_queue);

	struct thread* to = current_thread_node->thread;
	to->state = THREAD_RUNNING;
	log_printf("to %s\n", to->name);

	context_switch(from, to);
}

/*
 * called from time tick interrupt handler
 */
void sched_schedule(void)
{
	if (list_empty(&ready_queue))
		return;

	// kernel threads are non-interruptible
	if (current_thread_node->thread->type == KTHREAD)
		return;

	struct time current_time;
	time_get_current(&current_time);
	if (time_diff_ms(&current_time, &current_thread_start) > quantum) {
		if (current_thread_node) {
			current_thread_node->thread->state = THREAD_READY;
			current_thread_start = current_time;
			list_push_back(&ready_queue, current_thread_node);
		}

		sched_preempt();
	}
}

void sched_start(void)
{
	log_puts("sched_start()\n");

	kassert(current_thread_node == NULL);

	sched_preempt();
}

void sched_add_thread_node(struct thread_list_node* node)
{
	log_printf("sched add %s (%p)\n", node->thread->name, node->thread);

	node->thread->state = THREAD_READY;
	list_push_back(&ready_queue, node);
}

int sched_add_thread(struct thread* thread)
{
	kassert(thread != NULL);

	struct thread_list_node* const node = thread_list_node_create(thread);
	if (node)
		sched_add_thread_node(node);

	return (!node);
}

void sched_add_process(struct process* proc)
{
	struct thread_list_node* it;

	list_foreach(&proc->threads, it) {
		sched_add_thread(it->thread);
	}
}

struct thread* sched_get_current_thread(void)
{
	return current_thread_node->thread;
}

struct process* sched_get_current_process(void)
{
	return sched_get_current_thread()->process;
}

struct thread_list_node* sched_get_current_thread_node(void)
{
	return current_thread_node;
}

void sched_block_current_thread(void)
{
	log_printf("sched blocking %s\n", current_thread_node->thread->name);
	current_thread_node->thread->state = THREAD_BLOCKED;
	sched_preempt();
}

void sched_remove_current_thread(void)
{
	if (!current_thread_node)
		return;

	thread_destroy(current_thread_node->thread);
	current_thread_node = NULL;

	sched_preempt();
}

void sched_yield_current_thread(void)
{
	if (list_empty(&ready_queue))
		return;

	current_thread_node->thread->state = THREAD_READY;
	list_push_back(&ready_queue, current_thread_node);

	irq_disable();
	sched_preempt();
	irq_enable();
}

static void sched_timer_callback(void* data)
{
	struct thread_list_node* node = (struct thread_list_node*)data;
	sched_add_thread_node(node);
}

void sched_sleep_current_thread(unsigned int millis)
{
	current_thread_node->thread->state = THREAD_BLOCKED;

	struct timer* timer = timer_create(millis, sched_timer_callback, current_thread_node);
	kassert(timer != NULL);
	timer_register(timer);

	irq_disable();
	sched_preempt();
	irq_enable();
}

void sched_init(unsigned int quantum_in_ms)
{
	quantum = quantum_in_ms;
	list_init_null(&ready_queue);
}
