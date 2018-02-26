#include <kernel/cpu_context.h>
#include <kernel/errno.h>
#include <kernel/kmalloc.h>
#include <kernel/sched/sched.h>
#include <kernel/thread.h>
#include <kernel/usermode.h>
#include <libk/libk.h>

#define DEFAULT_STACK_SIZE 2048

static void thread_destroy(struct thread* thread)
{
	kfree(thread->cpu_context);
	if (thread->kstack.sp != (v_addr_t)NULL)
		kfree((void*)thread->kstack.sp);
	kfree(thread);
}

static int create_stack(v_addr_t* sp, size_t* size, size_t stack_size)
{
	void* stack = kmalloc(stack_size);
	if (!stack)
		return -ENOMEM;

	*sp = (v_addr_t)stack;
	*size = stack_size;

	return 0;
}

static int thread_create(const char* name, size_t stack_size,
						 start_func_t start, void* start_args,
						 enum thread_type type,
						 struct thread** result)
{
	int err;

	struct thread* thread = kmalloc(sizeof(struct thread));
	if (!thread)
		return -ENOMEM;

	memset(thread, 0, sizeof(struct thread));

	strncpy(thread->name, name, MAX_THREAD_NAME_LENGTH);

	err = create_stack(&thread->kstack.sp, &thread->kstack.size, stack_size);
	if (err)
		goto out_thread;

	thread->cpu_context = kmalloc(sizeof(struct cpu_context));
	if (!thread->cpu_context) {
		err = -ENOMEM;
		goto out_stack;
	}

	const v_addr_t stack_top = thread->kstack.sp + stack_size;

	cpu_context_create(thread->cpu_context, stack_top, (v_addr_t)start);
	cpu_context_pass_arg(thread->cpu_context, 2, 0);
	cpu_context_pass_arg(thread->cpu_context, 1, (v_addr_t)start_args);
	cpu_context_set_ret_ip(thread->cpu_context, (v_addr_t)NULL);

	thread->state = THREAD_CREATED;
	thread->type = type;

	thread->priority = SCHED_PRIORITY_LEVEL_DEFAULT;

	refcount_init(&thread->refcnt);

	*result = thread;
	return 0;

out_stack:
	kfree((void*)thread->kstack.sp);

out_thread:
	kfree(thread);

	return err;
}

int thread_kthread_create(const char* name, start_func_t start,
						  void* start_args, struct thread** result)
{
	return thread_create(name, DEFAULT_STACK_SIZE, start, start_args, KTHREAD,
						 result);
}

int thread_uthread_create(const char* name, start_func_t start,
						  void* start_args, struct thread** result)
{
	return thread_create(name, DEFAULT_STACK_SIZE, start, start_args, UTHREAD,
						 result);
}

void thread_ref(struct thread* thread)
{
	refcount_inc(&thread->refcnt);
}

void thread_unref(struct thread* thread)
{
	refcount_dec(&thread->refcnt);

	if (thread_get_ref(thread) == 0)
		thread_destroy(thread);
}

int thread_get_ref(const struct thread* thread)
{
	return refcount_get(&thread->refcnt);
}

thread_priority_t sched_get_priority(const struct thread* thread)
{
	return thread->priority;
}

int sched_set_priority(struct thread* thread, thread_priority_t priority)
{
	if (priority >= SCHED_PRIORITY_LEVEL_MIN &&
			priority <= SCHED_PRIORITY_LEVEL_MAX) {
		thread->priority = priority;
		return 0;
	}

	return -EINVAL;
}


void thread_yield(void)
{
	sched_yield_current_thread();
}

void thread_sleep(unsigned int millis)
{
	sched_sleep_current_thread(millis);
}
