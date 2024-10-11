[BITS 64]

global process_switch_asm
// void	process_switch(struct context **cur, struct context *next)
process_switch_asm:
	push rbx
	push rbp
	push r15
	push r14
	push r13
	push r12

	mov QWORD [rdi], rsp // set current context to rsp
	mov rsp, QWORD rsi // load new rsp

	// we're now on the next task stack
	pop r12
	pop r13
	pop r14
	pop r15
	pop rbp
	pop rbx

	ret
