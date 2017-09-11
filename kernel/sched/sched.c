#include <kernel/context_switch.h>
#include <kernel/kassert.h>
#include <kernel/libk.h>
#include <kernel/locking/spinlock.h>
#include <kernel/sched/idle.h>
#include <kernel/sched/sched.h>
#include <kernel/time/time.h>
#include <kernel/time/timer.h>

#include <kernel/log.h>

static unsigned int quantum;

// current thread
static struct thread_list_node* current_thread_node = NULL;
static struct time current_thread_start = { .sec = 0, .nano_sec = 0 };

static struct thread_list_synced ready_queue;


static void preempt(void)
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

	if (list_locked(&ready_queue))
		return;

	struct time current_time;
	time_get_current(&current_time);
	if (time_diff_ms(&current_time, &current_thread_start) > quantum) {
		if (current_thread_node) {
			current_thread_node->thread->state = THREAD_READY;
			current_thread_start = current_time;
			list_push_back(&ready_queue, current_thread_node);
		}

		preempt();
	}
}

/*
 * add
 */
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


/*
 * accessors
 */
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


/*
 * sched operations
 */
void sched_yield_current_thread(void)
{
	if (list_empty(&ready_queue))
		return;

	list_lock_synced(&ready_queue);

	current_thread_node->thread->state = THREAD_READY;
	list_push_back(&ready_queue, current_thread_node);

	list_unlock_synced(&ready_queue);

	preempt();
}

void sched_block_current_thread(void)
{
	log_printf("sched blocking %s\n", current_thread_node->thread->name);

	list_lock_synced(&ready_queue);

	current_thread_node->thread->state = THREAD_BLOCKED;

	list_unlock_synced(&ready_queue);

	preempt();
}

void sched_remove_current_thread(void)
{
	if (!current_thread_node)
		return;

	list_lock_synced(&ready_queue);

	thread_destroy(current_thread_node->thread);
	current_thread_node = NULL;

	list_unlock_synced(&ready_queue);

	preempt();
}

static void sched_timer_callback(void* data)
{
	list_lock_synced(&ready_queue);

	struct thread_list_node* node = (struct thread_list_node*)data;
	sched_add_thread_node(node);

	list_unlock_synced(&ready_queue);
}

void sched_sleep_current_thread(unsigned int millis)
{
	list_lock_synced(&ready_queue);

	current_thread_node->thread->state = THREAD_BLOCKED;

	struct timer* timer = timer_create(millis, sched_timer_callback, current_thread_node);
	kassert(timer != NULL);
	timer_register(timer);

	list_unlock_synced(&ready_queue);

	preempt();
}


/*
 * start, init
 */
void sched_start(void)
{
	log_puts("sched_start()\n");

	kassert(current_thread_node == NULL);

	preempt();
}

void sched_init(unsigned int quantum_in_ms)
{
	quantum = quantum_in_ms;
	list_init_null(&ready_queue);

	idle_init();
}
