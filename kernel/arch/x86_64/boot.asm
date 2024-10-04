#define ASM_FILE
#include "kernel/multiboot2.h"
#include "kernel/arch/x86_64/gdt.h"
#include "kernel/arch/x86_64/paging.h"
#include "kernel/mm/layout.h"

%define MULTIBOOT_FLAGS (0x0)
%define MULTIBOOT_SIZE (multiboot_header_end - multiboot_header)
%define VGA_BUFFER 0xb80a0 ; start of 2nd line

section .multiboot
multiboot_header:
	dd MULTIBOOT2_HEADER_MAGIC
	dd MULTIBOOT_ARCHITECTURE_I386
	dd MULTIBOOT_SIZE
	dd -(MULTIBOOT2_HEADER_MAGIC + MULTIBOOT_SIZE + MULTIBOOT_ARCHITECTURE_I386)
	memory_map_request:
	dw MULTIBOOT_HEADER_TAG_INFORMATION_REQUEST ; info request
	dw 0 ; info is required
	dd 16 ; size
	dd MULTIBOOT_TAG_TYPE_MMAP
	dd MULTIBOOT_TAG_TYPE_ACPI_OLD
	; dd MULTIBOOT_TAG_TYPE_ACPI_NEW ; TODO: support the ACPI 2.0, for the moment it needs more qemu configuration
	align 8 ; required end tag
	dw MULTIBOOT_TAG_TYPE_END
	dw 0
	dd 8
multiboot_header_end:

[BITS 32]
section .entry_32
; 1GB per page hehe
; two page identity mapping PDP_LOW
; two pages direct mapping PDP_MID
; two page mapping higher half to lower half PDP_HIGH
align 16 ; 16 bytes alignement follow the System V AMD64 ABI (actually we can choose not to)
stack_bottom_32:
resb 16384 ; same as dd but with uninitilized data
stack_top_32:

global PML4
align 4096 ; AMD64 volume 2 - 5.4.1 Field Definitions
PML4:
	dq (PDP_LOW + PAGE_RW + PAGE_PRESENT)
	times 256 dq (PDP_MID + PAGE_RW + PAGE_PRESENT)
	times 254 dq 0 ; PDP_MID will be added at runtime
	dq (PDP_HIGH + PAGE_RW + PAGE_PRESENT)
align 4096
PDP_MID: ; init setup for direct mapping | TODO: support more than 2gb
	dq PAGE_PRESENT | PAGE_RW | BOOT_PAGE_FLAG
	dq BOOT_PAGE_SIZE_IN_BYTES | PAGE_PRESENT | PAGE_RW | BOOT_PAGE_FLAG
	times 510 dq 0
PDP_LOW: ; should be used only for page references
	dq PAGE_PRESENT | PAGE_RW | BOOT_PAGE_FLAG
	dq BOOT_PAGE_SIZE_IN_BYTES | PAGE_PRESENT | PAGE_RW | BOOT_PAGE_FLAG
	times 510 dq 0
align 4096
PDP_HIGH: ; setup for kernel
	times 510 dq 0
	dq 0 | PAGE_PRESENT | PAGE_RW | BOOT_PAGE_FLAG
	dq BOOT_PAGE_SIZE_IN_BYTES | PAGE_PRESENT | PAGE_RW | BOOT_PAGE_FLAG

extern gdt ; defined in gdt.c
GDTPTR_32:
	dw GDT_SIZE * 8
	dq gdt - KERNEL_VMA ; addr is higher half

error_msg: db "HARDWARE NOT SUPPORTED", 0 ; TODO: set precise message

global _entry_32
_entry_32:
	cli ; clear all interrupts

	mov edi, eax ; first C argument
	mov esi, ebx ; second C argument
	mov esp, stack_top_32

	; is the cpuid instructions supported
	pushfd
	pop eax
	mov edx, eax
	xor eax, 1 << 21

	; try to flip bit
	push eax
	popfd

	; get result
	pushfd
	pop eax

	; restore default eflags setup
	push edx
	popfd

	cmp eax, edx
	je _hardware_not_supported

	; cpuid instruction is present https://www.felixcloutier.com/x86/cpuid

	mov eax, 0x80000000 ; Extended Function CPUID Information
	cpuid
	cmp eax, 0x80000001
	jl _hardware_not_supported

	; cpuid extended function available
	mov eax, 0x80000001
	cpuid

	; if bit 29 from the EDX register is set, hardware support 64-bit
	test edx, 1 << 29
	jz _hardware_not_supported

	; if bit 26 is set, hardware supports 1GB pages
	test edx, 1 << 26
	jz _hardware_not_supported

	; hardware is 64-bit ready
	mov eax, cr4
	or eax, 1 << 5 ; enable PAE
	or eax, 1 << 7 ; enable GLOBAL PAGES
	mov cr4, eax

	; set long mode bit in EFER MSR
	mov ecx, 0xC0000080
	rdmsr
	or eax, 1 << 8
	wrmsr

	; load PML4 to cr3
	mov eax, PML4
	mov cr3, eax

	; Enable paging and enter long mode
	mov eax, cr0
	or eax, 1 << 31
	mov cr0, eax

	; Load 64-bit GDT | switch from compatibility mode to long mode
	lgdt [GDTPTR_32]

	; Jump to 64-bit code
	jmp 0x8:_entry_64

_hardware_not_supported:
	; TODO: print something to screen
	mov esi, error_msg
	call _vga_print
	jmp _idle

_vga_print: ; string to print in esi
	mov al, 15 ; white
	mov edi, VGA_BUFFER

	_print_loop:
	mov BYTE al, [esi]
	; TODO: cleaner would be to use lodsb and stosw, maybe would loose in readability though?
	test al, al
	jz _print_end

	mov WORD [edi], ax

	inc esi
	add edi, 2
	jmp _print_loop

	_print_end:
	ret

_idle:
	cli
	halt:
	hlt
	jmp halt

[BITS 64]
section .entry_64
_entry_64:
	mov rsp, stack_top_64
	; call _gdt_reload
	; call _gdt_load_asm // reload gdt with pointer in higher half since paging applies
	extern kmain
	mov rax, kmain
	call rax

; stack should be linked with C kernel code
section .bss

global stack_top_64
align 16 ; 16 bytes alignement follow the System V AMD64 ABI (actually we can choose not to)
stack_bottom_64:
resb 16384 ; same as dd but with uninitilized data
stack_top_64: