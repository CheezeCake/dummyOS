#define ASSEMBLY

#include "segment.h"

.macro set_kernel_data_segment_registers tmp_reg=%ax
	movw $make_segment_selector(PRIVILEGE_KERNEL, KDATA), \tmp_reg
	movw \tmp_reg, %ds
	movw \tmp_reg, %es
	movw \tmp_reg, %fs
	movw \tmp_reg, %gs
	movw \tmp_reg, %ss
.endm

/* see struct cpu_context definition in cpu_context.c */
.macro cpu_context_save
	subl $2, %esp
	pushw %ss
	pushw %gs
	pushw %fs
	pushw %es
	pushw %ds

	pushl %ebp
	pushl %edi
	pushl %esi

	pushl %edx
	pushl %ecx
	pushl %ebx
	pushl %eax

	/* function prologue */
	movl $0, %ebp
.endm

.macro cpu_context_restore
	popl %eax
	popl %ebx
	popl %ecx
	popl %edx

	popl %esi
	popl %edi
	popl %ebp

	popw %ds
	popw %es
	popw %fs
	popw %gs
	popw %ss
	addl $6, %esp
.endm

.macro interrupt_enter
	cpu_context_save
	/* setup kernel segment register values */
	set_kernel_data_segment_registers
.endm

.macro interrupt_leave
	cpu_context_restore

	iret
.endm

.macro interrupt_call_c_handler nr, handlers_array
	pushl %esp
	pushl $\nr

	leal \handlers_array, %edi
	call *\nr * 4(%edi) /* call C handler */

	addl $8, %esp
.endm

/*
	build_handlers_table test_, 0, 2
	=> .long test_0
	   .long test_1
	   .long test_2
*/
.macro build_handlers_table prefix, from, to
	.long \prefix\from
	.if \to-\from
		.altmacro
		build_handlers_table \prefix, %(\from+1), \to
	.endif
.endm
