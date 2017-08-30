# see kernel/syscall.c
.extern syscall_table

.globl syscall_handler

.text

# eax =  syscall number
# ebx =  arg1
# ecx =  arg2
# edx =  arg3
# esi =  arg4
# edi =  arg5
# ebp =  arg6
syscall_handler:
	pushl %ebp
	pushl %edi
	pushl %esi
	pushl %edx
	pushl %ecx
	pushl %ebx

	leal syscall_table, %edi
	call *(%edi, %eax, 4)

	popl %ebx
	popl %ecx
	popl %edx
	popl %esi
	popl %edi
	popl %ebp

	iret
