#include <kernel/cpu_context.h>
#include <kernel/kmalloc.h>
#include <kernel/sched/sched.h>
#include <kernel/thread.h>
#include <kernel/usermode.h>
#include <libk/libk.h>

static int create_stack(v_addr_t* sp, size_t* size, size_t  stack_size)
{
	if (!(*sp = (v_addr_t)kmalloc(stack_size)))
		return -1;
	*size = stack_size;

	return 0;
}

static int thread_create(struct thread* thread, const char* name,
		struct process* proc, size_t stack_size, start_func_t start,
		void* start_args, exit_func_t exit, enum thread_type type)
{
	memset(thread, 0, sizeof(struct thread));

	strncpy(thread->name, name, MAX_THREAD_NAME_LENGTH);

	if (create_stack(&thread->stack.sp, &thread->stack.size, stack_size) == -1)
		return -1;

	if (!(thread->cpu_context = kmalloc(sizeof(struct cpu_context)))) {
		kfree((void*)thread->stack.sp);
		return -1;
	}

	thread->process = proc;

	const v_addr_t stack_top = thread->stack.sp + stack_size;

	*((v_addr_t*)stack_top - 1) = 0;
	*((v_addr_t*)stack_top - 2) = (v_addr_t)start_args; // FIXME: make this ABI independent
	*((v_addr_t*)stack_top - 3) = (v_addr_t)exit;
	cpu_context_create(thread->cpu_context,
			(v_addr_t)((v_addr_t*)stack_top - 3), (v_addr_t)start);

	thread->state = THREAD_READY;
	thread->type = type;

	return 0;
}

int thread_kthread_create(struct thread* thread, const char* name,
		struct process* proc, size_t stack_size, start_func_t start,
		void* start_args, exit_func_t exit)
{
	return thread_create(thread, name, proc, stack_size, start, start_args,
			exit, KTHREAD);
}

int thread_uthread_create(struct thread* thread, const char* name,
		struct process* proc, size_t stack_size, size_t kstack_size,
		start_func_t start, void* start_args, exit_func_t exit)
{
	// TODO: replace with real usermode start function pointer
	start_func_t usermode_entry_func = usermode_entry;

	// user function and arguments
	void** usermode_entry_args = kmalloc(2 * sizeof(void*));
	if (!usermode_entry_args)
		return -1;
	usermode_entry_args[0] = start;
	usermode_entry_args[1] = start_args;

	const int ret = thread_create(thread, name, proc, stack_size,
			usermode_entry_func, usermode_entry_args, exit, UTHREAD);
	if (create_stack(&thread->kstack.sp, &thread->kstack.size, kstack_size) == -1) {
		thread_destroy(thread);
		return -1;
	}

	return ret;
}

void thread_destroy(struct thread* thread)
{
	kfree(thread->cpu_context);
	kfree((void*)thread->stack.sp);
	if (thread->kstack.sp != (v_addr_t)NULL)
		kfree((void*)thread->kstack.sp);
}

void thread_yield(void)
{
	sched_yield_current_thread();
}

void thread_sleep(unsigned int millis)
{
	sched_sleep_current_thread(millis);
}
