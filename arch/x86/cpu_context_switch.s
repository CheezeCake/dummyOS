.global cpu_context_switch

# void cpu_context_switch(struct cpu_context* from, const struct cpu_context* to)
cpu_context_switch:
	cmpl $0, 4(%esp)
	je load

	pushl %eax
	movl %ebp, %eax
	movl 8(%esp), %ebp # ebp = from
	movl %eax, 28(%ebp) # save ebp
	popl %eax

	# save the rest of the current context in "from"
	movl %eax, (%ebp)
	movl %ebx, 4(%ebp)
	movl %ecx, 8(%ebp)
	movl %edx, 12(%ebp)

	movl %edi, 16(%ebp)
	movl %esi, 20(%ebp)
	movl %esp, 24(%ebp)

	movw %cs, 32(%ebp)
	movw %ds, 34(%ebp)
	movw %es, 36(%ebp)
	movw %fs, 38(%ebp)
	movw %gs, 40(%ebp)
	movw %ss, 42(%ebp)

	movl $return, 44(%ebp) # eip = (%esp) = return value

	# save eflags
	pushfl
	movl (%esp), %eax
	addl $4, %esp
	movl %eax, 48(%ebp)

load:
	# load "to" context
	movl 8(%esp), %ebp # ebp = to
	movl (%ebp), %eax
	movl 4(%ebp), %ebx
	movl 8(%ebp), %ecx
	movl 12(%ebp), %edx

	movl 16(%ebp), %edi
	movl 20(%ebp), %esi
	movl 24(%ebp), %esp # restore stack

	movw 34(%ebp), %ds
	movw 36(%ebp), %es
	movw 38(%ebp), %fs
	movw 40(%ebp), %gs
	movw 42(%ebp), %ss


	pushl 48(%ebp) # eflags
	pushw $0 # cs high
	pushw 32(%ebp) # cs
	pushl 44(%ebp) # push eip on the restored stack

	movl 28(%ebp), %ebp

	iret

return:
	retl
