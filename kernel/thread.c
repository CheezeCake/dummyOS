#include <kernel/cpu_context.h>
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
		return -1;
	*size = stack_size;

	return 0;
}

static struct thread* thread_create(const char* name,
		struct process* proc, size_t stack_size, start_func_t start,
		void* start_args, exit_func_t exit, enum thread_type type)
{
	struct thread* thread = kmalloc(sizeof(struct thread));
	if (!thread)
		return NULL;

	memset(thread, 0, sizeof(struct thread));

	strncpy(thread->name, name, MAX_THREAD_NAME_LENGTH);

	if (create_stack(&thread->stack.sp, &thread->stack.size, stack_size) == -1) {
		kfree(thread);
		return NULL;
	}

	if (!(thread->cpu_context = kmalloc(sizeof(struct cpu_context)))) {
		kfree((void*)thread->stack.sp);
		kfree(thread);
		return NULL;
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

	thread->priority = 0;

	refcount_init(&thread->refcnt);

	return thread;
}

struct thread* thread_kthread_create(const char* name,
		struct process* proc, size_t stack_size, start_func_t start,
		void* start_args, exit_func_t exit)
{
	return thread_create(name, proc, stack_size, start, start_args,
			exit, KTHREAD);
}

struct thread* thread_uthread_create(const char* name,
		struct process* proc, size_t stack_size, size_t kstack_size,
		start_func_t start, void* start_args, exit_func_t exit)
{
	// TODO: replace with real usermode start function pointer
	start_func_t usermode_entry_func = usermode_entry;

	// user function and arguments
	void** usermode_entry_args = kmalloc(2 * sizeof(void*));
	if (!usermode_entry_args)
		return NULL;
	usermode_entry_args[0] = start;
	usermode_entry_args[1] = start_args;

	struct thread* thread = thread_create(name, proc, stack_size,
			usermode_entry_func, usermode_entry_args, exit, UTHREAD);
	if (!thread)
		return NULL;

	if (create_stack(&thread->kstack.sp, &thread->kstack.size, kstack_size) == -1) {
		thread_destroy(thread);
		return NULL;
	}

	return thread;
}

void thread_ref(struct thread* thread)
{
	refcount_inc(&thread->refcnt);
}

void thread_unref(struct thread* thread)
{
	refcount_dec(&thread->refcnt);

	if (refcount_get(&thread->refcnt) == 0)
		thread_destroy(thread);
}

int thread_get_ref(const struct thread* thread)
{
	return refcount_get(&thread->refcnt);
}

void thread_yield(void)
{
	sched_yield_current_thread();
}

void thread_sleep(unsigned int millis)
{
	sched_sleep_current_thread(millis);
}
