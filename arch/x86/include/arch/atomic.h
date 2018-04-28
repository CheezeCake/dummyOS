#ifndef _ARCH_ATOMIC_H_
#define _ARCH_ATOMIC_H_

static inline void atomic_int_init(volatile atomic_int_t* v, int i)
{
	atomic_int_store(v, i);
}

static inline int atomic_int_load(const volatile atomic_int_t* v)
{
	return *v;
}

static inline void atomic_int_store(volatile atomic_int_t* v, int i)
{
	__asm__ volatile ("movl %1, (%0)" : : "r" (v), "r" (i));
}

#define __atomic_single_operand(instr, value) \
	__asm__ volatile ("lock "#instr" (%0)" : : "r" (value))

static inline void atomic_int_inc(volatile atomic_int_t* v)
{
	__atomic_single_operand(incl, v);
}

static inline void atomic_int_dec(volatile atomic_int_t* v)
{
	__atomic_single_operand(decl, v);
}

static inline int xadd(volatile atomic_int_t* v, int x)
{
	int ret = x;

	__asm__ volatile ("lock xaddl %0, (%1)"
					  : "+r" (ret)
					  : "r" (v)
					  : "memory", "cc");

	return ret;
}

#define atomic_int_add_return(v, x) (xadd((v), (x)) + (x))

static inline int atomic_int_inc_return(volatile atomic_int_t* v)
{
	return atomic_int_add_return(v, 1);
}

static inline int atomic_int_dec_return(volatile atomic_int_t* v)
{
	return atomic_int_add_return(v, -1);
}

#endif
