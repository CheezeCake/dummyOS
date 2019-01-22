#include <dummyos/compiler.h>
#include <kernel/interrupt.h>
#include <kernel/kassert.h>
#include <kernel/mm/uaccess.h>
#include <kernel/sched/sched.h>
#include <kernel/time/time.h>
#include <kernel/time/timer.h>
#include <libk/libk.h>

#include <kernel/log.h>

static struct timespec tick_value;
static struct timespec current = { .tv_sec = 0, .tv_nsec = 0 };

static LIST_DEFINE(timer_list);

static void time_update_timer_list(void);

void time_init(struct timespec tick_val)
{
	kassert(tick_val.tv_nsec < TIME_SEC_IN_NS);
	memcpy(&tick_value, &tick_val, sizeof(struct timespec));
}

void time_tick(void)
{
#ifndef NDEBUG
	time_t old_sec = current.tv_sec;
#endif

	timespec_add(&current, &tick_value);
	time_update_timer_list();

#ifndef NDEBUG
	// should be printed every second
	if (current.tv_sec != old_sec)
		log_printf("sec = %llu, nsec = %lu\n", current.tv_sec, current.tv_nsec);
#endif
}

void time_get_current(struct timespec* time)
{
	memcpy(time, &current, sizeof(struct timespec));
}

int64_t time_cmp(const struct timespec* t1, const struct timespec* t2)
{
	const int64_t sec_cmp = t1->tv_sec - t2->tv_sec;
	return (sec_cmp == 0) ? (t1->tv_nsec - t2->tv_nsec) : sec_cmp;
}

static void release_timer(list_node_t* n)
{
	struct timer* timer = list_entry(n, struct timer, t_list);
	timer_trigger(timer);
}

static void time_update_timer_list(void)
{
	list_node_t* it;
	list_node_t* next;

	irq_disable();

	list_foreach_safe(&timer_list, it, next) {
		const struct timer* timer_it = list_entry(it, struct timer, t_list);
		if (time_cmp(&current, &timer_it->time) >= 0) {
			list_erase(it);
			release_timer(it);
		}
	}

	irq_enable();
}

void time_add_timer(struct timer* timer)
{
	if (timer) {
		const struct thread* thr = container_of(timer, struct thread, timer);
		log_printf("sleep add %s\n", thr->name);

		irq_disable();
		list_push_back(&timer_list, &timer->t_list);
		irq_enable();
	}
}

void time_nanosleep_intr(struct thread* thr)
{
	struct timespec* timeout = &thr->timer.time;
	struct timespec* __user remainder =
		(struct timespec*)cpu_context_get_syscall_arg_2(thr->syscall_ctx);

	if (remainder) {
		timespec_diff(timeout, &current);
		copy_to_user(remainder, timeout, sizeof(struct timespec));
	}
}

int sys_nanosleep(const struct timespec* __user timeout,
		  struct timespec* __user remainder)
{
	struct timespec ktimeout;
	int err;

	err = copy_from_user(&ktimeout, timeout, sizeof(struct timespec));
	if (err)
		return err;

	if (remainder) {
		err = memset_user(remainder, 0, sizeof(struct timespec));
		if (err)
			return err;
	}

	sched_nanosleep(&ktimeout);

	return 0;
}
