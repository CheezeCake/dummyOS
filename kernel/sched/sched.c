#include <kernel/context_switch.h>
#include <kernel/kassert.h>
#include <kernel/locking/spinlock.h>
#include <kernel/sched/idle.h>
#include <kernel/sched/sched.h>
#include <kernel/time/time.h>
#include <kernel/time/timer.h>
#include <libk/libk.h>

#include <kernel/log.h>

#define PRIORITY_LEVELS 5

typedef struct thread_list sched_queue_t;

/* static unsigned int quantums[PRIORITY_LEVELS] = { 100, 80, 60, 40, 20 }; */
static thread_priority_t quantums[PRIORITY_LEVELS] = { 1000, 1000, 1000, 1000, 1000 };
#define get_thread_quantum(thread) quantums[(thread)->priority]

// current thread
static struct thread_list_node* current_thread_node = NULL;
static struct time current_thread_start = { .sec = 0, .nano_sec = 0 };

static sched_queue_t ready_queues[PRIORITY_LEVELS];
#define get_thread_queue(thread) ready_queues[(thread)->priority]

static spinlock_declare_lock(access_lock);


static inline thread_priority_t first_non_empty_queue_priority(void)
{
	thread_priority_t i = 0;
	while (list_empty(&ready_queues[i]) && i < PRIORITY_LEVELS)
			++i;
	return i;
}

static inline bool idle(void)
{
	return (first_non_empty_queue_priority() >= PRIORITY_LEVELS);
}

static void preempt(void)
{
	const thread_priority_t p = first_non_empty_queue_priority();
	kassert(p < PRIORITY_LEVELS); // every queue is empty
	sched_queue_t* ready_queue = &ready_queues[p];


	struct thread* from = (current_thread_node) ? current_thread_node->thread : NULL;
	log_printf("switching from %s ", (from) ? from->name : NULL);

	current_thread_node = list_front(ready_queue);
	list_pop_front(ready_queue);

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
	// kernel threads are non-interruptible
	if (current_thread_node->thread->type == KTHREAD)
		return;

	if (spinlock_locked(access_lock))
		return;

	struct time current_time;
	time_get_current(&current_time);
	if (time_diff_ms(&current_time, &current_thread_start) > get_thread_quantum(current_thread_node->thread)) {
		if (current_thread_node) {
			current_thread_start = current_time;

			current_thread_node->thread->state = THREAD_READY;
			list_push_back(&get_thread_queue(current_thread_node->thread), current_thread_node);
		}

		preempt();
	}
}

/*
 * utils
 */
static void free_node(struct thread_list_node* node)
{
	thread_unref(node->thread);
	kfree(node);
}


/*
 * add
 */
static void add_thread_node(struct thread_list_node* node)
{
	log_printf("sched add %s (%p)\n", node->thread->name, node->thread);

	thread_ref(node->thread);
	node->thread->state = THREAD_READY;

	list_push_back(&get_thread_queue(node->thread), node);
}

int sched_add_thread(struct thread* thread)
{
	kassert(thread != NULL);

	struct thread_list_node* const node = thread_list_node_create(thread);
	if (node)
		add_thread_node(node);

	return (!node);
}

static int add_process_threads(struct thread_list_node* it)
{
	if (!it)
		return 0;

	if (sched_add_thread(it->thread) != 0)
		return -1;

	if (add_process_threads(list_it_next(it)) != 0) {
		sched_remove_thread(it->thread);
		return -1;
	}

	return 0;
}

int sched_add_process(const struct process* proc)
{
	return add_process_threads(list_begin(&proc->threads));
}


/*
 * remove
 */
int sched_remove_thread(struct thread* thread)
{
	sched_queue_t* ready_queue = &get_thread_queue(thread);
	struct thread_list_node* it;

	list_foreach(ready_queue, it) {
		if (it->thread == thread) {
			list_erase(ready_queue, it);
			free_node(it);

			return 0;
		}
	}

	return -1;
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


/*
 * sched operations
 */
void sched_yield_current_thread(void)
{
	if (idle())
		return;

	spinlock_lock(access_lock);

	current_thread_node->thread->state = THREAD_READY;
	list_push_back(&get_thread_queue(current_thread_node->thread), current_thread_node);

	spinlock_unlock(access_lock);

	preempt();
}

void sched_block_current_thread(void)
{
	log_printf("sched_block %s\n", current_thread_node->thread->name);

	spinlock_lock(access_lock);

	struct thread* current_thread = current_thread_node->thread;
	current_thread->state = THREAD_BLOCKED;

	kassert(thread_get_ref(current_thread) > 1 &&
			"blocking thread without taking ownership");
	free_node(current_thread_node);

	spinlock_unlock(access_lock);

	preempt();
}

void sched_remove_current_thread(void)
{
	spinlock_lock(access_lock);

	thread_unref(current_thread_node->thread);
	current_thread_node = NULL;

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
	log_printf("sched_sleep %s\n", current_thread_node->thread->name);

	spinlock_lock(access_lock);

	current_thread_node->thread->state = THREAD_SLEEPING;

	struct timer* timer = timer_create(millis, sched_timer_callback,
			current_thread_node->thread);
	kassert(timer != NULL);
	timer_register(timer);

	// don't unref the thread, the ownership is transfered to the timer
	kfree(current_thread_node);

	spinlock_unlock(access_lock);

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

void sched_init()
{
	for (unsigned int i = 0; i < PRIORITY_LEVELS; ++i)
		list_init_null(&ready_queues[i]);

	idle_init();
}
