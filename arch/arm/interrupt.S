/* see struct cpu_context definition in cpu_context.c */
.macro cpu_context_save
	sub sp, sp, #4 /* alignement */
	stmfd sp, {sp, lr}^
	sub sp, sp, #8
	push {r0-r12}
.endm

.macro cpu_context_restore
	mov r0, sp
	bl thread_switch_setup

	pop {r0-r12}
	ldmfd sp, {sp, lr}^
	add sp, sp, #8
	add sp, sp, #4 /* alignement */
.endm
