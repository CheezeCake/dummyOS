#ifndef _KLOG_H_
#define _KLOG_H_

#ifdef ARCH_x86
#include "../arch/x86/log.h"
#endif

#ifndef log_printf
#define log_printf(format, ...)
#endif

#ifndef log_puts
#define log_puts(str)
#endif

#endif
