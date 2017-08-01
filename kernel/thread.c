#include <kernel/thread.h>
#include <kernel/cpu_context.h>
#include <kernel/kmalloc.h>
#include <kernel/libk.h>
#include <kernel/sched.h>

int thread_create(struct thread* thread, const char* name, size_t stack_size,
		void (*start)(void), void (*exit)(void))
{
	memset(thread, 0, sizeof(struct thread));

	strncpy(thread->name, name, MAX_THREAD_NAME_LENGTH);

	if (!(thread->stack = (v_addr_t)kmalloc(stack_size)))
		return -1;
	thread->stack_size = stack_size;

	if (!(thread->cpu_context = kmalloc(sizeof(struct cpu_context)))) {
		kfree((void*)thread->stack);
		return -1;
	}

	const v_addr_t stack_top = thread->stack + stack_size;
	*((v_addr_t*)stack_top - 1) = (v_addr_t)exit;
	cpu_context_create(thread->cpu_context, (v_addr_t)((v_addr_t*)stack_top - 1), (v_addr_t)start);

	thread->state = THREAD_READY;

	return 0;
}

void thread_destroy(struct thread* thread)
{
	kfree(thread->cpu_context);
	kfree((void*)thread->stack);
}

void thread_yield(void)
{
	sched_yield_current_thread();
}

void thread_sleep(unsigned int millis)
{
	sched_sleep_current_thread(millis);
}
