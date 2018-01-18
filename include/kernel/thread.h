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
typedef unsigned int thread_priority_t;

#define MAX_THREAD_NAME_LENGTH 16

enum thread_state
{
	THREAD_CREATED,
	THREAD_RUNNING,
	THREAD_READY,
	THREAD_BLOCKED,
	THREAD_SLEEPING,
	THREAD_ZOMBIE
};
enum thread_type { KTHREAD, UTHREAD };

struct thread
{
	char name[MAX_THREAD_NAME_LENGTH];

	struct cpu_context* cpu_context;

	struct process* process;

	// kernel stack
	struct
	{
		v_addr_t sp;
		size_t size;
	} kstack;

	enum thread_state state;
	enum thread_type type;

	thread_priority_t priority;

	refcount_t refcnt;

	struct list_node p_thr_list; /**< Chained in process::threads */
	struct list_node s_ready_queue; /**< Chained in sched::@ref ::ready_queues */
	struct list_node sem_wq; /**< Chained in sem_t::wait_queue */

	wait_queue_entry_t wqe; // wait_queue entry
};

int thread_kthread_create(const char* name, size_t stack_size,
						  start_func_t start, void* start_args,
						  struct thread** result);
int thread_uthread_create(const char* name, size_t kstack_size,
						  start_func_t start, void* start_args,
						  struct thread** result);

void thread_ref(struct thread* thread);
void thread_unref(struct thread* thread);
int thread_get_ref(const struct thread* thread);

void thread_yield(void);
void thread_sleep(unsigned int millis);

thread_priority_t thread_get_priority(void);
int thread_set_priority(thread_priority_t priority);

#endif
