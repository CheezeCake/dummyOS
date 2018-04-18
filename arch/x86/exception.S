#define ASSEMBLY

#include "exception.h" /* EXCEPTION codes */
#include "interrupt.S" /* common macros */

/* see exception.c */
.extern exception_handlers


.text

/* No error code exception handlers */
.irp nr,									\
	EXCEPTION_DIVIDE_ERROR,					\
	EXCEPTION_DEBUG,						\
	EXCEPTION_NMI_INTERRUPT,				\
	EXCEPTION_BREAKPOINT,					\
	EXCEPTION_OVERFLOW,						\
	EXCEPTION_BOUND_RANGE_EXCEDEED,			\
	EXCEPTION_INVALID_OPCODE,				\
	EXCEPTION_DEVICE_NOT_AVAILABLE,			\
	EXCEPTION_COPROCESSOR_SEGMENT_OVERRUN,	\
	EXCEPTION_RESERVED_1,					\
	EXCEPTION_FLOATING_POINT_ERROR,			\
	EXCEPTION_MACHINE_CHECK,				\
	EXCEPTION_RESERVED_2,					\
	EXCEPTION_RESERVED_3,					\
	EXCEPTION_RESERVED_4,					\
	EXCEPTION_RESERVED_5,					\
	EXCEPTION_RESERVED_6,					\
	EXCEPTION_RESERVED_7,					\
	EXCEPTION_RESERVED_8,					\
	EXCEPTION_RESERVED_9,					\
	EXCEPTION_RESERVED_10,					\
	EXCEPTION_RESERVED_11,					\
	EXCEPTION_RESERVED_12,					\
	EXCEPTION_RESERVED_13,					\
	EXCEPTION_RESERVED_14

.align 4
asm_exception_handler_\nr:
.type asm_interrupt_handler_\nr, @function
	pushl $0 /* error code */
	interrupt_enter

	interrupt_call_c_handler \nr, exception_handlers

	interrupt_leave
.endr

/* Error code exception handlers */
.irp nr,							\
	EXCEPTION_DOUBLE_FAULT,			\
	EXCEPTION_INVALID_TSS,			\
	EXCEPTION_SEGMENT_NOT_PRESENT,	\
	EXCEPTION_STACK_SEGMENT_FAULT,	\
	EXCEPTION_GENERAL_PROTECTION,	\
	EXCEPTION_PAGE_FAULT,			\
	EXCEPTION_ALIGNEMENT_CHECK

.align 4
asm_exception_handler_\nr:
.type asm_exception_handler_\nr, @function
	interrupt_enter

	interrupt_call_c_handler \nr, exception_handlers

	interrupt_leave
.endr


.section .rodata

.global asm_exception_handlers

.align 4
asm_exception_handlers:
	build_handlers_table asm_exception_handler_, EXCEPTION_MIN, EXCEPTION_MAX
