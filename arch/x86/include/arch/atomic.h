#ifndef _ARCH_ATOMIC_H_
#define _ARCH_ATOMIC_H_

static inline void atomic_int_init(volatile atomic_int_t* v, int i)
{
	atomic_int_store(v, i);
}

static inline int atomic_int_load(const volatile atomic_int_t* v)
{
	int value;
	__asm__ volatile ("movl (%1), %%eax\n"
					  "movl %%eax, %0"
					  : "=r" (value)
					  : "r" (v)
					  : "%eax", "memory");

	return value;
}

static inline void atomic_int_store(volatile atomic_int_t* v, int i)
{
	__asm__ volatile ("movl %1, (%0)\n" : : "r" (v), "r" (i) : "memory");
}

#define __atomic_single_operand(instr, value) \
	__asm__ volatile ("lock "#instr" (%0)" : : "r" (value) : "memory")

static inline void atomic_int_inc(volatile atomic_int_t* v)
{
	__atomic_single_operand(incl, v);
}

static inline void atomic_int_dec(volatile atomic_int_t* v)
{
	__atomic_single_operand(decl, v);
}

#endif
