#ifndef _KERNEL_THREAD_H_
#define _KERNEL_THREAD_H_

#include <stddef.h>
#include <arch/cpu_context.h>
#include <kernel/types.h>
#include <kernel/time/time.h>
#include <kernel/list.h>

struct process;

typedef void (*start_func_t)(void* data);
typedef void (*exit_func_t)(void);

#define MAX_THREAD_NAME_LENGTH 16

enum thread_state { THREAD_RUNNING, THREAD_READY, THREAD_BLOCKED };
enum thread_type { KTHREAD, UTHREAD };

struct thread
{
	char name[MAX_THREAD_NAME_LENGTH];

	struct cpu_context* cpu_context;

	struct process* process;

	struct {
		v_addr_t sp;
		size_t size;
	} stack, kstack;

	enum thread_type type;

	enum thread_state state;
};

int thread_kthread_create(struct thread* thread, const char* name,
		struct process* proc, size_t stack_size, start_func_t start,
		void* start_args, exit_func_t exit);
int thread_uthread_create(struct thread* thread, const char* name,
		struct process* proc, size_t stack_size, size_t kstack_size,
		start_func_t start, void* start_args, exit_func_t exit);
void thread_destroy(struct thread* thread);

void thread_yield(void);
void thread_sleep(unsigned int millis);

#endif
