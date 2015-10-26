#include <kernel/thread.h>
#include <kernel/cpu_context.h>
#include <kernel/kmalloc.h>
#include <kernel/libk.h>

int thread_create(struct thread* thread, const char* name, size_t stack_size,
		void (start(void)), void (exit(void)))
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
	*((uint32_t*)stack_top - 1) = (uint32_t)exit;
	cpu_context_create(thread->cpu_context, stack_top - 4, (v_addr_t)start);

	return 0;
}

void thread_destroy(struct thread* thread)
{
	kfree(thread->cpu_context);
	kfree((void*)thread->stack);
}