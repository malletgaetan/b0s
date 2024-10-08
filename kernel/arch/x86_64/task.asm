[BITS 64]

global task_switch_asm
// void	task_switch(u64 *cur_rsp, u64 *next_rsp)
task_switch_asm:
	push rbx
	push rbp
	push r15
	push r14
	push r13
	push r12

	mov QWORD [rdi], rsp // save rsp to old
	mov rsp, QWORD [rsi] // load new rsp

	// we're now on the next task stack
	pop r12
	pop r13
	pop r14
	pop r15
	pop rbp
	pop rbx

	ret
