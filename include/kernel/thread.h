#ifndef _KERNEL_THREAD_H_
#define _KERNEL_THREAD_H_

#include <dummyos/const.h>
#include <kernel/cpu_context.h>
#include <kernel/sched/wait.h>
#include <kernel/time/time.h>
#include <kernel/types.h>
#include <libk/list.h>
#include <libk/refcount.h>

struct process;

typedef unsigned int thread_priority_t;

#define DEFAULT_KSTACK_SIZE PAGE_SIZE

enum thread_state
{
	THREAD_RUNNING,
	THREAD_READY,
	THREAD_SLEEPING,
	THREAD_DEAD
};

enum thread_type
{
	KTHREAD,
	UTHREAD
};

struct stack
{
	v_addr_t sp;
	size_t size;
};

struct thread
{
	char* name;

	union
	{
		struct process* process;
		void* kthr_data;
	};

	struct cpu_context* cpu_context;

	// kernel stack
	struct stack kstack;

	enum thread_state state;
	enum thread_type type;

	thread_priority_t priority;

	refcount_t refcnt;

	list_node_t p_thr_list; /**< Chained in process::threads */
	list_node_t s_ready_queue; /**< Chained in sched::@ref ::ready_queues */
	list_node_t sem_wq; /**< Chained in sem_t::wait_queue */

	wait_queue_entry_t wqe; /**< Chained in wait_queue_t::threads */
};

int __kthread_create(char* name, v_addr_t start, struct thread** result);
int thread_create(v_addr_t start, v_addr_t stack, struct thread** result);
int thread_clone(const struct thread* thread, char* name,
				 struct thread** result);

void thread_ref(struct thread* thread);
void thread_unref(struct thread* thread);
int thread_get_ref(const struct thread* thread);

void thread_set_state(struct thread* thread, enum thread_state state);
enum thread_state thread_get_state(const struct thread* thread);

struct cpu_context* thread_switch_setup(struct thread* thread,
										struct thread* prev);

v_addr_t thread_get_kstack_top(const struct thread* thread);

#endif
