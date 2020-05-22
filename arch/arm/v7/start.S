#define ASSEMBLY

#include <arch/memory.h>
#include <kernel/mm/vm.h>
#include "../exception.h"
#include "paging.h"

.data

.p2align 5
__boot_ttbr0_lv1:
.word DESC_BLOCK | AF | AP_KERNEL | AP_RW | ATTR_MEM, 0
.word 0, 0
.word 0, 0
.word 0, 0

.p2align 12
__boot_ttbr1_lv3:
.skip LV3_ENTRIES * 8, 0

.global __boot_ttbr1_lv2
.global __boot_ttbr1_lv2_phys

.p2align 12
__boot_ttbr1_lv2:
.skip LV2_ENTRIES * 8, 0

.set __boot_ttbr0_lv1_phys, __boot_ttbr0_lv1 + 0x8000 - KERNEL_SPACE_START
.set __boot_ttbr1_lv2_phys, __boot_ttbr1_lv2 + 0x8000 - KERNEL_SPACE_START
.set __boot_ttbr1_lv3_phys, __boot_ttbr1_lv3 + 0x8000 - KERNEL_SPACE_START

__stack_bottom:
.skip 0x2000 /* 8KB */
__stack_top:

.section .text.boot, "x"

.global _start

_start:
	/* keep CORE0 alive */
	mrc p15, #0, r3, c0, c0, #5 /* CPUID register */
	and r3, r3, #3 /* CPU ID field */
	cmp r3, #0
	bne halt

	/* get out of hypervisor mode if needed */
	mrs r3, cpsr
	and r4, r3, #ARM_MODE_MASK
	cmp r4, #ARM_MODE_HYP
	bne not_in_hyp_mode
	bic r3, r3, #ARM_MODE_MASK
	orr r3, r3, #ARM_MODE_SVC
	msr spsr_cxsf, r3
	add lr, pc, #4 /* not_in_hyp_mode relative address */
	msr ELR_hyp, lr
	eret
not_in_hyp_mode:

#if 0
	/* switch to secure mode */
	mrc p15, 0, r3, c1, c1, 0 /* secure configuration register */
	and r3, r3, #1
	cmp r3, #0 /* check NS bit */
	beq 1f
	bic r3, r3, #1 /* clear NS bit */
	mcr p15, 0, r3, c1, c1, 0
#endif
1:
	/* Clear out bss. */
	ldr r4, =__bss_start
	ldr r9, =__bss_end
	mov r5, #0
	mov r6, #0
	mov r7, #0
	mov r8, #0
	b 3f
2:
	/* store multiple at r4. */
	stmia r4!, {r5-r8}

	/* If we are still below bss_end, loop. */
3:
	cmp r4, r9
	blo 2b

paging:
	/* setup paging */

	/* TTBR1 LV2 entry */
	ldr r1, =__boot_ttbr1_lv2_phys
	ldr r2, =__boot_ttbr1_lv3_phys
	orr r2, r2, #DESC_TABLE
	orr r2, r2, #AF
	str r2, [r1]

	/* TTBR1 LV3 entries */
	ldr r1, =__boot_ttbr1_lv3_phys
	ldr r2, =__kernel_p_start
	add r4, r1, #PAGE_SIZE /* r4 = lv3 table end */
1:
	orr r2, r2, #DESC_PAGE
	orr r2, r2, #AF
	str r2, [r1], #8
	add r2, r2, #PAGE_SIZE
	cmp r1, r4
	blo 1b

write_ttbcr:
	/* MAIR0, attr0 */
	mov r1, #0xff /* normal memory */
	mcr p15, #0, r1, c10, c2, #0

	/* TTBCR */
	ldr r1, =0x80020000 /* EAE, T0SZ = 0, T1SZ = 2 (3:1 split) */
	mcr p15, #0, r1, c2, c0, #2
	isb

	/* TTBR0 */
	ldr r1, =__boot_ttbr0_lv1_phys
	mcr p15, #0, r1, c2, c0, #0

	/* TTBR1 */
	ldr r1, =__boot_ttbr1_lv2_phys
	mcr p15, #0, r1, c2, c0, #1

enable_mmu:
	dsb ish
	isb

	mrc p15, #0, r1, c1, c0, #0
	orr r1, r1, #1 /* enable MMU */
	mcr p15, #0, r1, c1, c0, #0

calling_main:
	/* setup stack */
	ldr sp, =__stack_top
	sub sp, sp, #8

	/* stack trace sentinel */
	mov r3, #0
	push {r3} /* r11/fp */
	push {r3} /* lr */

	/* call the C main function */
	ldr r3, =__kernel_main
	blx r3

halt:
	wfe
	b halt
