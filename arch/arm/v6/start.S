#define ASSEMBLY

#include "../exception.h"

.section .text.boot, "x"

.global _start

_start:
	/* Clear out bss. */
	ldr r4, =__bss_start
	ldr r9, =__bss_end
	mov r5, #0
	mov r6, #0
	mov r7, #0
	mov r8, #0
	b 2f
1:
	/* store multiple at r4. */
	stmia r4!, {r5-r8}

	/* If we are still below bss_end, loop. */
2:
	cmp r4, r9
	blo 1b

	/* call the C main function */
	ldr r3, =__kernel_main
	blx r3
