[BITS 64]
; section .text
extern interrupt_handler
section .text

; BASIC INTERRUPT HANDLERS
; ARRAY OF POINTER TO EACH INTERRUPT HANDLER
global idt_vectors
idt_vectors:
%assign i 0
%rep 256
	dq _idt_interrupt_handler_asm%+i
%assign i i+1
%endrep

// HANDLER FOR VECTOR WITH ERROR CODE
%macro INTERRUPT_WITH_ERROR 1
global _idt_interrupt_handler_asm%1
_idt_interrupt_handler_asm%1:
	push %1 ; error code is the last element of the stack, so [rsp]
	jmp _general_handler_asm
%endmacro

// HANDLER FOR VECTOR WITHOUT ERROR CODE
%macro INTERRUPT_WITHOUT_ERROR 1
global _idt_interrupt_handler_asm%1
_idt_interrupt_handler_asm%1:
	push 0
	push %1
	jmp _general_handler_asm
%endmacro

_general_handler_asm:
	push rax
	push rbx
	push rcx
	push rdx
	push rsi
	push rdi
	push rbp
	push r8
	push r9
	push r10
	push r11
	push r12
	push r13
	push r14
	push r15

	cld // TODO: apparently it can change I have no idea when though
	mov rdi, rsp
	call interrupt_handler
	mov rsp, rax ; return pointer to rsp, could also use rdi

	pop r15
	pop r14
	pop r13
	pop r12
	pop r11
	pop r10
	pop r9
	pop r8
	pop rbp
	pop rdi
	pop rsi
	pop rdx
	pop rcx
	pop rbx
	pop rax

	add rsp, 16 ; remove erro code and vector number
	iretq

// AMD64 Volume 2 - page 247
INTERRUPT_WITHOUT_ERROR 0
INTERRUPT_WITHOUT_ERROR 1
INTERRUPT_WITHOUT_ERROR 2
INTERRUPT_WITHOUT_ERROR 3
INTERRUPT_WITHOUT_ERROR 4
INTERRUPT_WITHOUT_ERROR 5
INTERRUPT_WITHOUT_ERROR 6
INTERRUPT_WITHOUT_ERROR 7
INTERRUPT_WITH_ERROR 8
INTERRUPT_WITHOUT_ERROR 9
INTERRUPT_WITH_ERROR 10
INTERRUPT_WITH_ERROR 11
INTERRUPT_WITH_ERROR 12
INTERRUPT_WITH_ERROR 13
INTERRUPT_WITH_ERROR 14
INTERRUPT_WITHOUT_ERROR 15
INTERRUPT_WITHOUT_ERROR 16
INTERRUPT_WITH_ERROR 17
INTERRUPT_WITHOUT_ERROR 18
INTERRUPT_WITHOUT_ERROR 19
INTERRUPT_WITHOUT_ERROR 20
INTERRUPT_WITHOUT_ERROR 21
INTERRUPT_WITHOUT_ERROR 22
INTERRUPT_WITHOUT_ERROR 23
INTERRUPT_WITHOUT_ERROR 24
INTERRUPT_WITHOUT_ERROR 25
INTERRUPT_WITHOUT_ERROR 26
INTERRUPT_WITHOUT_ERROR 27
INTERRUPT_WITHOUT_ERROR 28
INTERRUPT_WITHOUT_ERROR 29
INTERRUPT_WITH_ERROR 30
INTERRUPT_WITHOUT_ERROR 31

%assign i 32
%rep 224
	INTERRUPT_WITHOUT_ERROR i
%assign i i+1
%endrep