#ifndef _KLOG_H_
#define _KLOG_H_

#ifdef ARCH_x86
#include "../arch/x86/log.h"
#endif

#ifndef LOG_PRINTF
#define LOG_PRINTF(format, ...)
#endif

#ifndef LOG_PUTS
#define LOG_PUTS(str)
#endif

#endif
