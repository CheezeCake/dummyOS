#ifndef _KERNEL_THREAD_H_
#define _KERNEL_THREAD_H_

#include <stddef.h>

#include <arch/cpu_context.h>
#include <kernel/time/time.h>
#include <kernel/types.h>
#include <libk/list.h>
#include <libk/refcount.h>
#include <kernel/sched/wait.h>

struct process;

typedef void (*start_func_t)(void* data);
typedef void (*exit_func_t)(void);
typedef unsigned int thread_priority_t;

#define MAX_THREAD_NAME_LENGTH 16

enum thread_state
{
	THREAD_RUNNING,
	THREAD_READY,
	THREAD_BLOCKED,
	THREAD_SLEEPING,
	THREAD_DEAD
};
enum thread_type { KTHREAD, UTHREAD };

struct thread
{
	char name[MAX_THREAD_NAME_LENGTH];

	struct cpu_context* cpu_context;

	struct process* process;

	// kernel stack and user stack
	struct
	{
		v_addr_t sp;
		size_t size;
	} stack, kstack;

	enum thread_state state;
	enum thread_type type;

	thread_priority_t priority;

	refcount_t refcnt;

	struct list_node p_thr_list; // process.threads node
	struct list_node s_ready_queue; // sched ready_queue node
	struct list_node sem_wq; // sem wait_queue node

	wait_queue_entry_t wqe; // wait_queue entry
};

struct thread* thread_kthread_create(const char* name,
		struct process* proc, size_t stack_size, start_func_t start,
		void* start_args, exit_func_t exit);
struct thread* thread_uthread_create(const char* name,
		struct process* proc, size_t stack_size, size_t kstack_size,
		start_func_t start, void* start_args, exit_func_t exit);

void thread_ref(struct thread* thread);
void thread_unref(struct thread* thread);
int thread_get_ref(const struct thread* thread);

void thread_yield(void);
void thread_sleep(unsigned int millis);

#endif
