#include <kernel/errno.h>
#include <kernel/interrupt.h>
#include <kernel/kassert.h>
#include <kernel/sched/idle.h>
#include <kernel/sched/sched.h>
#include <kernel/time/time.h>
#include <kernel/time/timer.h>
#include <libk/libk.h>

#include <kernel/log.h>

typedef thread_list_t sched_queue_t;
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

#if 0
static void ready_queues_dump(void)
{
	for (int i = 0; i < SCHED_PRIORITY_LEVELS; ++i) {
		list_node_t* it;
		log_printf("[%d]: ", i);
		list_foreach(&ready_queues[i], it) {
			struct thread* thr = list_entry(it, struct thread, s_ready_queue);
			log_printf("%s(%p)state=%d | ", thr->name, (void*)thr, thr->state);
		}
		log_putchar('\n');
	}
}
#endif

static inline thread_priority_t first_non_empty_queue_priority(void)
{
	thread_priority_t i = SCHED_PRIORITY_LEVEL_MAX;
	while (list_empty(&ready_queues[i]) && i > 0)
			--i;
	return i;
}

static inline bool idling(void)
{
	return (first_non_empty_queue_priority() == SCHED_PRIORITY_LEVEL_MIN);
}

static void preempt_current(void)
{
	cpu_context_preempt();
}

static inline bool preemptible(struct cpu_context* cpu_ctx)
{
	// threads in kernel mode are non-preemptible
	return cpu_context_is_usermode(cpu_ctx);
}

/*
 * called from time tick interrupt handler
 */
void sched_tick(struct cpu_context* interrupted_ctx)
{
}

static bool quatum_expired(struct thread* thr, const struct time* start)
{
	struct time current_time;
	const quantum_ms_t quantum = get_thread_quantum(thr);

	time_get_current(&current_time);

	return (time_diff_ms(&current_time, start) > quantum);
}

static struct thread* next_thread(struct thread* prev_thr,
								  struct cpu_context* prev_ctx)
{
	const thread_priority_t p = first_non_empty_queue_priority();
	kassert(p < SCHED_PRIORITY_LEVELS); // every queue is empty
	sched_queue_t* ready_queue = &ready_queues[p];

	if (prev_thr)
		prev_thr->cpu_context = prev_ctx; // update cpu_context

	struct thread* next = get_thread_list_entry(list_front(ready_queue));
	list_pop_front(ready_queue);

	return next;
}

static inline void set_current_thread(struct thread* thr)
{
	struct time current_time;

	kassert(thr != NULL);

	time_get_current(&current_time);

	current_thread = thr;
	current_thread_start = current_time;

	thread_set_state(thr, THREAD_RUNNING);
}

static void log_sched_switch(const struct thread* from, const struct thread* to)
{
	log_printf("switching from %s (%p, pid=%d) ", (from) ? from ->name : NULL,
			   (void*)from, (from && from->process) ? from->process->pid : 0);
	log_printf("to %s (%p, pid=%d)\n", (to) ? to ->name : NULL,
			   (void*)to, (to && to->process) ? to->process->pid : 0);
}

struct cpu_context* sched_schedule_yield(struct cpu_context* cpu_ctx)
{
	struct thread* cur;
	struct thread* next;

	irq_disable();

	cur = current_thread;

	do {
		next = next_thread(current_thread, cpu_ctx);
	} while (process_signal_pending(next->process) && // deliver pending signal
			 signal_handle(next) != 0);

	log_sched_switch(cur, next);

	// set current thread
	if (current_thread && current_thread->state == THREAD_RUNNING) {
		sched_add_thread(current_thread);
		thread_unref(current_thread);
	}
	set_current_thread(next);

	irq_enable();

	return next->cpu_context;
}

struct cpu_context* sched_schedule(struct cpu_context* cpu_ctx)
{
	if (!current_thread ||
		(preemptible(cpu_ctx) &&
		 quatum_expired(current_thread, &current_thread_start)))
	{
		return sched_schedule_yield(cpu_ctx);
	}

	return cpu_ctx;
}

/*
 * add
 */
int sched_add_thread(struct thread* thread)
{
	irq_disable();

	kassert(thread != NULL);
	if (thread->state != THREAD_RUNNING)
		log_printf("%s(): %s (%p) state=%d\n", __func__, thread->name,
				   (void*)thread, thread->state);

	if (thread->state != THREAD_DEAD) {
		thread_set_state(thread, THREAD_READY);

		if (thread->type == UTHREAD && thread->process->state == PROC_LOCKED) {
			log_printf("%s(): process locked !\n", __func__);
		}
		else {
			list_push_back(&get_thread_queue(thread), &thread->s_ready_queue);
			thread_ref(thread);
		}
	}

	irq_enable();

	return 0;
}

int sched_add_process(struct process* proc)
{
	list_node_t* it;
	int n = 0;

	list_foreach(&proc->threads, it) {
		struct thread* thr = list_entry(it, struct thread, p_thr_list);
		int err = sched_add_thread(thr);
		if (err)
			return err;

		++n;
	}

	return n;
}

/*
 * remove
 */
int sched_remove_thread(struct thread* thread)
{
	int err = 0;

	irq_disable();

	if (list_node_chained(&thread->s_ready_queue)) {
		list_erase(&thread->s_ready_queue);
		thread_unref(thread);
	}
	else {
		err = -EINVAL;
	}

	irq_enable();

	return err;
}

int sched_remove_process(struct process* proc)
{
	list_node_t* it;
	int n = 0;

	list_foreach(&proc->threads, it) {
		struct thread* thr = list_entry(it, struct thread, p_thr_list);
		int err = sched_remove_thread(thr);
		if (!err)
			++n;
	}

	return n;
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
	return (current_thread->type == UTHREAD) ? current_thread->process : NULL;
}


/*
 * sched operations
 */
void sched_yield(void)
{
	if (idling())
		return;

	preempt_current();
}

void sched_exit(void)
{
	__irq_disable();

	thread_set_state(current_thread, THREAD_DEAD);
	thread_unref(current_thread);

	preempt_current();
}

static int sched_timer_callback(void* data)
{
	struct thread* thread = (struct thread*)data;
	sched_add_thread(thread);
	// timer drops ownership
	thread_unref(thread);

	return 0;
}

static void __sched_sleep()
{
	thread_set_state(current_thread, THREAD_SLEEPING);

	kassert(thread_get_ref(current_thread) > 1 &&
			"putting thread to sleep without taking ownership");
	thread_unref(current_thread);
}

void sched_sleep_millis(unsigned int millis)
{
	log_printf("%s(): %s (%p) state=%d\n", __func__, current_thread->name,
			   (void*)current_thread, current_thread->state);

	struct timer* timer = timer_create(millis, sched_timer_callback,
			current_thread);
	kassert(timer != NULL);
	thread_ref(current_thread); // the timer refererences the thread
	timer_register(timer);
	timer_unref(timer);

	__sched_sleep();

	preempt_current();
}

void sched_sleep_event(void)
{
	log_printf("%s(): %s (%p) state=%d\n", __func__, current_thread->name,
			   (void*)current_thread, current_thread->state);

	__sched_sleep();

	preempt_current();
}


/*
 * start, init
 */
void sched_start(void)
{
	log_puts("sched_start()\n");

	kassert(current_thread == NULL);
	preempt_current();
}

void sched_init()
{
	for (unsigned int i = 0; i < SCHED_PRIORITY_LEVELS; ++i)
		list_init(&ready_queues[i]);
}
