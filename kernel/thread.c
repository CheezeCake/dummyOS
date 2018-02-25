#include <kernel/cpu_context.h>
#include <kernel/errno.h>
#include <kernel/kmalloc.h>
#include <kernel/sched/sched.h>
#include <kernel/thread.h>
#include <kernel/usermode.h>
#include <libk/libk.h>

static void thread_destroy(struct thread* thread)
{
	kfree(thread->cpu_context);
	kfree((void*)thread->stack.sp);
	if (thread->kstack.sp != (v_addr_t)NULL)
		kfree((void*)thread->kstack.sp);
	kfree(thread);
}

static int create_stack(v_addr_t* sp, size_t* size, size_t  stack_size)
{
	if (!(*sp = (v_addr_t)kmalloc(stack_size)))
		return -ENOMEM;
	*size = stack_size;

	return 0;
}

static int thread_create(const char* name, size_t stack_size,
									start_func_t start, void* start_args,
									exit_func_t exit, enum thread_type type,
									struct thread** result)
{
	int err;
	struct thread* thread = kmalloc(sizeof(struct thread));
	if (!thread)
		return -ENOMEM;

	memset(thread, 0, sizeof(struct thread));

	strncpy(thread->name, name, MAX_THREAD_NAME_LENGTH);

	err = create_stack(&thread->stack.sp, &thread->stack.size, stack_size);
	if (err != 0)
		goto err_thread;

	if (!(thread->cpu_context = kmalloc(sizeof(struct cpu_context)))) {
		err =  -ENOMEM;
		goto err_stack;
	}

	const v_addr_t stack_top = thread->stack.sp + stack_size;

	cpu_context_create(thread->cpu_context, stack_top, (v_addr_t)start);
	cpu_context_pass_arg(thread->cpu_context, 2, 0);
	cpu_context_pass_arg(thread->cpu_context, 1, (v_addr_t)start_args);
	cpu_context_set_ret_ip(thread->cpu_context, (v_addr_t)exit);

	thread->state = THREAD_CREATED;
	thread->type = type;

	thread->priority = SCHED_PRIORITY_LEVEL_DEFAULT;

	refcount_init(&thread->refcnt);

	*result = thread;

	return 0;

	// error
err_stack:
	kfree((void*)thread->stack.sp);
err_thread:
	kfree(thread);

	return err;
}

int thread_kthread_create(const char* name, size_t stack_size,
						  start_func_t start, void* start_args,
						  exit_func_t exit, struct thread** result)
{
	return thread_create(name, stack_size, start, start_args,
			exit, KTHREAD, result);
}

int thread_uthread_create(const char* name, size_t stack_size,
						  size_t kstack_size, start_func_t start,
						  void* start_args, exit_func_t exit,
						  struct thread** result)
{
	int err;
	start_func_t usermode_entry_func = usermode_entry;

	// user function and arguments
	void** usermode_entry_args = kmalloc(2 * sizeof(void*));
	if (!usermode_entry_args)
		return -ENOMEM;
	usermode_entry_args[0] = start;
	usermode_entry_args[1] = start_args;

	struct thread* thread;
	err = thread_create(name, stack_size, usermode_entry_func,
						usermode_entry_args, exit, UTHREAD, &thread);
	if (err != 0)
		return err;

	err = create_stack(&thread->kstack.sp, &thread->kstack.size, kstack_size);
	if (err != 0) {
		thread_destroy(thread);
		return err;
	}

	*result = thread;

	return 0;
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
