#ifndef _ARCH_ATOMIC_H_
#define _ARCH_ATOMIC_H_

#include <kernel/interrupt.h>

static inline void atomic_int_init(volatile atomic_int_t* v, int32_t i)
{
	*v = i;
}

static inline int32_t atomic_int_load(const volatile atomic_int_t* v)
{
	uint32_t val;

	__asm__ volatile ("ldrex %0, [%1]" : "=r" (val) : "r" (v) : "cc", "memory");

	return val;
}

static inline void atomic_int_store(volatile atomic_int_t* v, int32_t i)
{
	__asm__ volatile ("1:	strex r2, %0, [%1]	\n"
			  "		cmp r2, #0	\n"
			  "		bne 1b		\n"
			  :
			  : "r" (i), "r" (v)
			  : "r2", "cc", "memory");
}

static inline int32_t atomic_int_add_return(volatile atomic_int_t* v, int32_t x)
{
	int32_t ret;

	__asm__ volatile ("1:  ldrex   %0, [%1]		\n"
			  "    add     %0, %0, %2	\n"
			  "    strex   r2, %0, [%1]	\n"
			  "    cmp     r2, #0		\n"
			  "    bne     1b		\n"
			  : "=&r" (ret)
			  : "r" (v), "r" (x)
			  : "r2", "cc", "memory");

	return ret;
}

static inline int32_t atomic_int_inc_return(volatile atomic_int_t* v)
{
	return atomic_int_add_return(v, 1);
}

static inline int32_t atomic_int_dec_return(volatile atomic_int_t* v)
{
	return atomic_int_add_return(v, -1);
}

static inline void atomic_int_inc(volatile atomic_int_t* v)
{
	atomic_int_inc_return(v);
}

static inline void atomic_int_dec(volatile atomic_int_t* v)
{
	atomic_int_dec_return(v);
}

#endif
