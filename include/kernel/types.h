#ifndef _KERNEL_TYPES_H_
#define _KERNEL_TYPES_H_

#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <arch/types.h>

typedef uintptr_t p_addr_t; // physical address
typedef uintptr_t v_addr_t; // virtual address

typedef long off_t;

typedef int pid_t;

#endif
