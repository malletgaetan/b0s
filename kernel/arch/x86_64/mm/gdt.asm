#define ASM_FILE
#include "kernel/arch/x86_64/mm/gdt.h"

global gdt_reload_asm
gdt_reload_asm:
	push rbp
	mov rbp, rsp

	lgdt [rdi]

	; Reload CS through far return
	push GDT_KERNEL_CODE_INDEX * 8
	lea rax, [rel _reload_cs]
	push rax
	retfq
_reload_cs:
	mov ax, GDT_KERNEL_DATA_INDEX * 8
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov ss, ax

	mov rsp, rbp
	pop rbp
	ret