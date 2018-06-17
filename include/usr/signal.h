#ifndef _USR_SIGNAL_H_
#define _USR_SIGNAL_H_

#define SIGHUP		1
#define SIGINT		2
#define SIGQUIT		3
#define SIGILL		4
#define SIGTRAP		5
#define SIGABRT		6
#define SIGBUS		7
#define SIGFPE		8
#define SIGKILL		9
#define SIGUSR1		10
#define SIGSEGV		11
#define SIGUSR2		12
#define SIGPIPE		13
#define SIGALRM		14
#define SIGTERM		15
#define SIGCHLD		17
#define SIGCONT		18
#define SIGSTOP		19

#define SIGNAL_MAX	19

#define SIG_DFL		((sighandler_t)0)
#define SIG_IGN		((sighandler_t)1)

#define SIG_ERR		((sighandler_t)-1)


typedef struct __sig_stack_t
{
	void* ss_sp;
	size_t ss_size;
	uint32_t ss_flags;
} stack_t;

union sigval
{
	uint32_t sival_int; /* Integer value */
	void* sival_ptr; /* Pointer value */
};

typedef struct siginfo
{
	uint32_t si_signo; /* Signal number */
	uint32_t si_code; /* Signal code */

	uint32_t si_errno; /* If non-zero, an errno value associated with
						  this signal, as defined in <errno.h> */

	pid_t si_pid; /* Sending process ID */
	/* uid_t si_uid; */ /* Real user ID of sending process */
	void* si_addr; /* Address of faulting instruction */
	uint32_t si_status; /* Exit value or signal */

	uint64_t si_band; /* Band event for SIGPOLL */

	union sigval si_value; /* Signal value */
} siginfo_t;


typedef void (*sighandler_t)(uint32_t sig);
typedef void (*sigrestore_t)(uint32_t sig, siginfo_t* info, void* ucontext);
typedef uint32_t sigset_t;

struct sigaction
{
	union
	{
		sighandler_t sa_handler;
		sigrestore_t sa_sigaction;
	};

	sigset_t sa_mask;

	uint32_t sa_flags;
};

/*
 * struct sigaction::s_flags flags
 */
#define SA_NODEFER		1 /* Causes signal not to be automatically blocked
							 on entry to signal handler */
#define SA_ONSTACK		2 /* Causes signal delivery to occur on an alternate
							 stack */
#define SA_RESETHAND	3 /* Causes signal dispositions to be set to SIG_DFL
							 on entry to signal handlers */
#define SA_SIGINFO		4 /* Causes extra information to be passed to signal
							 handlers at the time of receipt of a signal */

#endif
