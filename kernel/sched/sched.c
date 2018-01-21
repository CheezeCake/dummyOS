#include <kernel/context_switch.h>
#include <kernel/kassert.h>
#include <kernel/locking/spinlock.h>
#include <kernel/sched/idle.h>
#include <kernel/sched/sched.h>
#include <kernel/time/time.h>
#include <kernel/time/timer.h>
#include <libk/libk.h>

#include <kernel/log.h>

typedef struct thread_list sched_queue_t;
typedef unsigned int quantum_ms_t;

/* static unsigned int quantums[SCHED_PRIORITY_LEVELS] = { 100, 80, 60, 40, 20 }; */
static quantum_ms_t quantums[SCHED_PRIORITY_LEVELS] = { 1000, 1000, 1000, 1000, 1000 };
#define get_thread_quantum(thread) quantums[(thread)->priority]

// current thread
static struct thread* current_thread = NULL;
static struct time current_thread_start = { .sec = 0, .nano_sec = 0 };

static sched_queue_t ready_queues[SCHED_PRIORITY_LEVELS];
#define get_thread_queue(thread) ready_queues[(thread)->priority]
#define get_thread_list_entry(node) list_entry(node, struct thread, s_ready_queue)

static spinlock_t access_lock = SPINLOCK_NULL;


static inline thread_priority_t first_non_empty_queue_priority(void)
{
	thread_priority_t i = 0;
	while (list_empty(&ready_queues[i]) && i < SCHED_PRIORITY_LEVELS)
			++i;
	return i;
}

static inline bool idle(void)
{
	return (first_non_empty_queue_priority() >= SCHED_PRIORITY_LEVELS);
}

static void preempt(void)
{
	const thread_priority_t p = first_non_empty_queue_priority();
	kassert(p < SCHED_PRIORITY_LEVELS); // every queue is empty
	sched_queue_t* ready_queue = &ready_queues[p];


	struct thread* from = current_thread;
	log_printf("switching from %s ", (from) ? from->name : NULL);

	struct thread* to = get_thread_list_entry(list_front(ready_queue));
	to->state = THREAD_RUNNING;
	log_printf("to %s\n", to->name);

	current_thread = to;
	list_pop_front(ready_queue);

	context_switch(from, to);
}

/*
 * called from time tick interrupt handler
 */
void sched_schedule(void)
{
	// kernel threads are non-interruptible
	if (current_thread->type == KTHREAD)
		return;

	if (spinlock_locked(access_lock))
		return;

	struct time current_time;
	time_get_current(&current_time);
	const quantum_ms_t quantum = get_thread_quantum(current_thread);

	if (time_diff_ms(&current_time, &current_thread_start) > quantum) {
		current_thread_start = current_time;

		current_thread->state = THREAD_READY;
		list_push_back(&get_thread_queue(current_thread),
				&current_thread->s_ready_queue);

		preempt();
	}
}

/*
 * add
 */
int sched_add_thread(struct thread* thread)
{
	kassert(thread != NULL);

	log_printf("sched add %s (%p)\n", thread->name, thread);

	thread_ref(thread);
	thread->state = THREAD_READY;

	list_push_back(&get_thread_queue(thread), &thread->s_ready_queue);

	return 0;
}

static int add_process_threads(struct list_node* proc_thr_list_it)
{
	if (!proc_thr_list_it)
		return 0;

	struct thread* it_thread = list_entry(proc_thr_list_it, struct thread, p_thr_list);

	if (sched_add_thread(it_thread) != 0)
		return -1;

	if (add_process_threads(list_it_next(proc_thr_list_it)) != 0) {
		sched_remove_thread(it_thread);
		return -1;
	}

	return 0;
}

int sched_add_process(struct process* proc)
{
	int err = add_process_threads(list_begin(&proc->threads));
	if (err == 0)
		list_push_back(&process_list, &proc->p_list);

	return err;
}


/*
 * remove
 */
int sched_remove_thread(struct thread* thread)
{
	sched_queue_t* ready_queue = &get_thread_queue(thread);
	struct list_node* it;

	list_foreach(ready_queue, it) {
		struct thread* it_thread = get_thread_list_entry(it);
		if (it_thread == thread) {
			list_erase(ready_queue, it);
			thread_unref(it_thread);

			return 0;
		}
	}

	return -1;
}

int sched_remove_process(struct process* proc)
{
	int ret = 0;
	struct list_node* it;

	list_foreach(&proc->threads, it) {
		struct thread* thread = list_entry(it, struct thread, p_thr_list);
		if (sched_remove_thread(thread) != 0)
			ret = -1;
	}

	list_erase(&process_list, &proc->p_list);

	return ret;
}


/*
 * accessors
 */
struct thread* sched_get_current_thread(void)
{
	return current_thread;
}

struct process* sched_get_current_process(void)
{
	return sched_get_current_thread()->process;
}


/*
 * sched operations
 */
void sched_yield_current_thread(void)
{
	if (idle())
		return;

	spinlock_lock(access_lock);

	current_thread->state = THREAD_READY;
	list_push_back(&get_thread_queue(current_thread), &current_thread->s_ready_queue);

	spinlock_unlock(access_lock);

	preempt();
}

void sched_block_current_thread(void)
{
	log_printf("sched_block %s\n", current_thread->name);

	spinlock_lock(access_lock);

	current_thread->state = THREAD_BLOCKED;

	kassert(thread_get_ref(current_thread) > 1 &&
			"blocking thread without taking ownership");
	thread_unref(current_thread);

	spinlock_unlock(access_lock);

	preempt();
}

void sched_remove_current_thread(void)
{
	spinlock_lock(access_lock);

	thread_unref(current_thread);
	current_thread = NULL;

	spinlock_unlock(access_lock);

	preempt();
}

static int sched_timer_callback(void* data)
{
	spinlock_lock(access_lock);

	struct thread* thread = (struct thread*)data;
	sched_add_thread(thread);
	// timer drops ownership
	thread_unref(thread);

	spinlock_unlock(access_lock);

	return 0;
}

void sched_sleep_current_thread(unsigned int millis)
{
	log_printf("sched_sleep %s\n", current_thread->name);

	spinlock_lock(access_lock);

	current_thread->state = THREAD_SLEEPING;

	// don't unref the thread, the ownership is transfered to the timer
	struct timer* timer = timer_create(millis, sched_timer_callback,
			current_thread);
	kassert(timer != NULL);
	timer_register(timer);
	timer_unref(timer);

	spinlock_unlock(access_lock);

	preempt();
}


/*
 * start, init
 */
void sched_start(void)
{
	log_puts("sched_start()\n");

	kassert(current_thread == NULL);
	preempt();
}

void sched_init()
{
	for (unsigned int i = 0; i < SCHED_PRIORITY_LEVELS; ++i)
		list_init_null(&ready_queues[i]);
}
