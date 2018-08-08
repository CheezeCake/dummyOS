#ifndef _KERNEL_SIGNAL_H_
#define _KERNEL_SIGNAL_H_

#include <dummyos/const.h>
#include <dummyos/signal.h>
#include <kernel/types.h>
#include <kernel/process.h>
#include <libk/list.h>

struct process;
struct thread;

#define SIGNAL_MAX_PENDING 16

typedef struct __sig_stack_t
{
	void* ss_sp;
	size_t ss_size;
	int ss_flags;
} stack_t;

typedef struct queued_siginfo
{
	siginfo_t sinfo;
	sigset_t saved_mask;

	list_node_t s_queue;
} queued_siginfo_t;

struct signal_manager
{
	list_t sig_queue;

	sigset_t mask;
	struct sigaction actions[NSIG];

	stack_t altstack;
	list_t handled_stack;
};

int signal_init(struct signal_manager* sigm);

int signal_create(struct signal_manager** result);

void signal_copy(struct signal_manager* dst, const struct signal_manager* src);

void signal_destroy(struct signal_manager* sigm);

bool signal_pending(const struct signal_manager* sigm);

int signal_send(struct signal_manager* sigm, int sig, void* addr,
				const struct process* sender);

siginfo_t* signal_pop(struct signal_manager* sigm);

void signal_reset_dispositions(struct signal_manager* sigm);

int signal_handle(struct thread* thr);

bool signal_is_ign(int sig, const struct signal_manager* sigm);

bool signal_is_dlf(int sig, const struct signal_manager* sigm);

#endif
