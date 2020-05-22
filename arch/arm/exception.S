#define ASSEMBLY

#include <arch/memory.h>
#include "exception.h"
#include "interrupt.S"

/* syscall.S */
.extern syscall_handler

.global exception_vectors

.text

.p2align PAGE_SIZE_SHIFT
exception_vectors:
	ldr pc, _reset_handler
	ldr pc, _undefined_instruction_handler
	ldr pc, _software_interrupt_handler
	ldr pc, _prefetch_abort_handler
	ldr pc, _data_abort_handler
	nop
	ldr pc, _irq_handler
	ldr pc, _fast_irq_handler

_reset_handler:			.word asm_handler_reset
_undefined_instruction_handler: .word asm_handler_undefined_instruction
_software_interrupt_handler:	.word syscall_handler
_prefetch_abort_handler:	.word asm_handler_prefetch_abort
_data_abort_handler:		.word asm_handler_data_abort
_irq_handler:			.word asm_handler_irq
_fast_irq_handler:		.word asm_handler_fast_irq


.macro asm_exception_handler_prologue lr_offset
	sub lr, lr, #\lr_offset
	srsfd sp!, #ARM_MODE_SVC /* save lr and spsr to svc stack */
	cpsid if, #ARM_MODE_SVC  /* switch to svc mode with interrupts disabled */
	cpu_context_save

	mov r0, sp
.endm

.macro asm_exception_handler_epilogue
	cpu_context_restore
	rfefd sp!
.endm

.macro asm_exception_handler name, lr_offset, c_handler
asm_handler_\name:
	asm_exception_handler_prologue \lr_offset
	ldr r1, =\c_handler
	blx r1
	asm_exception_handler_epilogue
.endm

/*
 * exception handlers
 */
asm_exception_handler reset, 0, reset_handler
asm_exception_handler undefined_instruction, 4, undefined_instruction_handler
asm_exception_handler prefetch_abort, 4, prefetch_abort_handler
asm_exception_handler data_abort, 8, data_abort_handler
asm_exception_handler fast_irq, 4, fast_irq_handler

/* defined in arch.c: void (*arch_irq_handler)(void) */
.extern arch_irq_handler

asm_handler_irq:
	asm_exception_handler_prologue 4
	ldr r1, =arch_irq_handler
	ldr r1, [r1]
	blx r1

	mov r0, sp /* saved cpu_context */
	bl sched_schedule
	cmp r0, sp
	beq 1f /* same thread */
	mov sp, r0 /* swith to next thread context */
1:
	asm_exception_handler_epilogue


.global exception_set_mode_stack

/* void exception_set_mode_stack(unsigned mode, void* stack) */
exception_set_mode_stack:
	mrs r3, cpsr	/* save current mode */
	msr cpsr_c, r0	/* switch to mode */
	mov sp, r1	/* switch stack */
	msr cpsr_c, r3	/* switch back */
	bx lr
