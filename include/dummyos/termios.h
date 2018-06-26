#ifndef _DUMMYOS_TERMIOS_H_
#define _DUMMYOS_TERMIOS_H_

#include <stdint.h>

/* http://pubs.opengroup.org/onlinepubs/009695399/basedefs/termios.h.html */

typedef uint32_t cc_t;
typedef uint32_t speed_t;
typedef uint32_t tcflag_t;

/*
 * termios::c_cc size
 */
#define NCCS 11

/*
 * termios struct
 */
struct termios
{
	tcflag_t c_iflag;	/* Input modes. */
	tcflag_t c_oflag;	/* Output modes. */
	tcflag_t c_cflag;	/* Control modes. */
	tcflag_t c_lflag;	/* Local modes. */
	cc_t c_cc[NCCS];	/* Control characters */
};

/*
 * termios::c_cc subscripts
 */
#define VEOF	0
#define VEOL	1
#define VERASE	2
#define VINTR	3
#define VKILL	4
#define VMIN	5
#define VQUIT	6
#define VSTART	7
#define VSTOP	8
#define VSUSP	9
#define VTIME	10

/*
 * Input Modes (termios::c_iflag)
 */
#define BRKINT	(1 << 0)	/* Signal interrupt on break. */
#define ICRNL	(1 << 1)	/* Map CR to NL on input. */
#define IGNBRK	(1 << 2)	/* Ignore break condition. */
#define IGNCR	(1 << 3)	/* Ignore CR. */
#define IGNPAR	(1 << 4)	/* Ignore characters with parity errors. */
#define INLCR	(1 << 5)	/* Map NL to CR on input. */
#define INPCK	(1 << 6)	/* Enable input parity check. */
#define ISTRIP	(1 << 7)	/* Strip character. */
#define IXANY	(1 << 8)	/* Enable any character to restart output. */
#define IXOFF	(1 << 9)	/* Enable start/stop input control. */
#define IXON	(1 << 10)	/* Enable start/stop output control. */
#define PARMRK	(1 << 11)	/* Mark parity errors. */

/*
 * Ouput Modes (termios::c_oflag)
 */
#define OPOST		(1 << 0)			/* Post-process output. */
#define ONLCR		(1 << 1)			/* Map NL to CR-NL on output. */
#define OCRNL		(1 << 2)			/* Map CR to NL on output. */
#define ONOCR		(1 << 3)			/* No CR output at column 0. */
#define ONLRET		(1 << 4)			/* NL performs CR function. */
#define OFILL		(1 << 5)			/* Use fill characters for delay. */
#define NLDLY		(1 << 6)			/* Select newline delays: */
#define		NL0		(0)					/* Newline type 0. */
#define		NL1		(1 << 6)			/* Newline type 1. */
#define CRDLY		(1 << 7 | 1 << 8)	/* Select carriage-return delays: */
#define		CR0		(0)					/* Carriage-return delay type 0. */
#define		CR1		(1 << 7)			/* Carriage-return delay type 1. */
#define		CR2		(1 << 8)			/* Carriage-return delay type 2. */
#define		CR3		(1 << 7 | 1 << 8)	/* Carriage-return delay type 3. */
#define TABDLY		(1 << 9 | 1 << 10)	/* Select horizontal-tab delays: */
#define		TAB0	(0)					/* Horizontal-tab delay type 0. */
#define		TAB1	(1 << 9)			/* Horizontal-tab delay type 1. */
#define		TAB2	(1 << 10)			/* Horizontal-tab delay type 2. */
#define		TAB3	(1 << 9 | 1 << 10)	/* Expand tabs to spaces. */
#define BSDLY		(1 << 11)			/* Select backspace delays: */
#define		BS0		(0)					/* Backspace-delay type 0. */
#define		BS1		(1 << 11)			/* Backspace-delay type 1. */
#define VTDLY		(1 << 12)			/* Select vertical-tab delays: */
#define		VT0		(0)					/* Vertical-tab delay type 0. */
#define		VT1		(1 << 12)			/* Vertical-tab delay type 1. */
#define FFDLY		(1 << 13)			/* Select form-feed delays: */
#define		FF0		(0)					/* Form-feed delay type 0. */
#define		FF1		(1 << 13)			/* Form-feed delay type 1. */

/*
 * Local Modes
 * termios::clflag
 */
#define ECHO	(1 << 0) /* Enable echo. */
#define ECHOE	(1 << 1) /* Echo erase character as error-correcting backspace. */
#define ECHOK	(1 << 2) /* Echo KILL. */
#define ECHONL	(1 << 3) /* Echo NL. */
#define ICANON	(1 << 4) /* Canonical input (erase and kill processing). */
#define IEXTEN	(1 << 5) /* Enable extended input character processing. */
#define ISIG	(1 << 6) /* Enable signals. */
#define NOFLSH	(1 << 7) /* Disable flush after interrupt or quit. */
#define TOSTOP	(1 << 8) /* Send SIGTTOU for background output. */

speed_t cfgetispeed(const struct termios *);
speed_t cfgetospeed(const struct termios *);
int cfsetispeed(struct termios *, speed_t);
int cfsetospeed(struct termios *, speed_t);
int tcdrain(int);
int tcflow(int, int);
int tcflush(int, int);
int tcgetattr(int, struct termios *);

pid_t tcgetsid(int);

int tcsendbreak(int, int);
int tcsetattr(int, int, const struct termios *);

#endif
