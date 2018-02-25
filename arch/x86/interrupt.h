#ifndef _INTERRUPT_H_
#define _INTERRUPT_H_

#ifdef ASSEMBLY

#define SAVE_CONTEXT()	\
	pushal ;			\
	pushw %ss ;			\
	pushw %ds ;			\
	pushw %es ;			\
	pushw %fs ;			\
	pushw %gs

#define RESTORE_CONTEXT()	\
	popw %gs ;				\
	popw %fs ;				\
	popw %es ;				\
	popw %ds ;				\
	popw %ss ;				\
	popal					\

#define SET_DATA_SEGMENT_REGISTERS(dpl, index)					\
	movw $make_segment_selector(PRIVILEGE_KERNEL, KDATA), %ax;	\
	movw %ax, %ss ;												\
	movw %ax, %ds ;												\
	movw %ax, %es ;												\
	movw %ax, %fs ;												\
	movw %ax, %gs

#endif // ! ASSEMBLY

#endif
