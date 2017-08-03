#ifndef _ARCH_ATOMIC_H_
#define _ARCH_ATOMIC_H_

#define __atomic_single_operand(instr, value)	\
	__asm__ __volatile__ (						\
			#instr" %0"						\
			:									\
			: "m" (value)						\
			: "memory")

#define atomic_dec_int(value) __atomic_single_operand(decl, value)

#define atomic_inc_int(value) __atomic_single_operand(incl, value)

#endif
