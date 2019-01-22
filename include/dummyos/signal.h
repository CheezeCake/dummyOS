#ifndef _DUMMYOS_SIGNAL_H_
#define _DUMMYOS_SIGNAL_H_

/* http://pubs.opengroup.org/onlinepubs/009695399/basedefs/signal.h.html */

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
#define NSIG		20 /* 0 implied */

#ifdef __KERNEL__
# define SIG_DFL	((sighandler_t)0)
# define SIG_IGN	((sighandler_t)1)
# define SIG_ERR	((sighandler_t)-1)
#endif


union sigval
{
	int sival_int; /* Integer value */
	void* sival_ptr; /* Pointer value */
};

typedef struct siginfo
{
	int si_signo; /* Signal number */
	int si_code; /* Signal code */

	int si_errno; /* If non-zero, an errno value associated with
			 this signal, as defined in <errno.h> */

	pid_t si_pid; /* Sending process ID */
	/* uid_t si_uid; */ /* Real user ID of sending process */
	void* si_addr; /* Address of faulting instruction */
	int si_status; /* Exit value or signal */

	long si_band; /* Band event for SIGPOLL */

	union sigval si_value; /* Signal value */
} siginfo_t;

typedef unsigned long sigset_t;

/*
 * sigprocmask
 */
#define SIG_SETMASK		0		/* Set mask with sigprocmask() */
#define SIG_BLOCK		1		/* Set of signals to block */
#define SIG_UNBLOCK		2		/* Set of signals to unblock */


typedef void (*sighandler_t)(int sig);
typedef void (*sigrestore_t)(int sig, siginfo_t* info, void* ucontext);

struct sigaction
{
	union
	{
		sighandler_t sa_handler;
		sigrestore_t sa_sigaction;
	};

	sigset_t sa_mask;

	int sa_flags;
};

/*
 * struct sigaction::s_flags flags
 */
#define SA_NODEFER		(1 << 0) /* Causes signal not to be automatically
					    blocked on entry to signal handler */
#define SA_ONSTACK		(1 << 1) /* Causes signal delivery to occur on an
					    alternate stack */
#define SA_RESETHAND	(1 << 2) /* Causes signal dispositions to be set to
				    SIG_DFL on entry to signal handlers */
#define SA_SIGINFO		(1 << 3) /* Causes extra information to be passed to
					    signal handlers at the time of receipt of a
					    signal */
#define SA_RESTART		(1 << 4) /* Causes certain functions to become
					    restartable */

#endif
