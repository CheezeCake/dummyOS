#ifndef _LOG_H_
#define _LOG_H_

#include "drivers/ioport_0xe9.h"

#define	log_printf(format, ...) ioport_0xe9_printf(format, __VA_ARGS__)
#define	log_puts(str) ioport_0xe9_puts(str)

#endif
