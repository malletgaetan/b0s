#ifndef IDT_H
# define IDT_H

# include "kernel/types.h"

#define IDT_SIZE 256

# define VECTOR_PRESENT 0b10000000
# define INT_GATE 0b1110
# define TRAP_GATE 0b1111

enum {
	IDT_USER = 0x60,
	IDT_TRAP = 0xef,
	IDT_GATE = 0x8e
};

enum {
	IDT_DIVIDE_BY_ZERO = 0,
	IDT_DEBUG = 1,
	IDT_NMI = 2,
	IDT_BREAKPOINT = 3,
	IDT_OVERFLOW = 4,
	IDT_BOUND_RANGE_EXCEED = 5,
	IDT_INVALID_OPCODE = 6,
	IDT_DEV_NOT_AVL = 7,
	IDT_DOUBLE_FAULT = 8,
	IDT_COPROC_SEG_OVERRUN = 9,
	IDT_INVALID_TSS = 10,
	IDT_SEGMENT_NOT_PRESENT = 11,
	IDT_STACK_SEGMENT_FAULT = 12,
	IDT_GENERAL_PROTECTION = 13,
	IDT_PAGE_FAULT = 14,
	IDT_FLOATING_POINT_ERR = 16,
	IDT_ALIGNMENT_CHECK = 17,
	IDT_MACHINE_CHECK = 18,
	IDT_SIMD_FP_EXC = 19,
	IDT_CONTROL_PROTECTION_ERROR = 21,
	IDT_HYPERVISOR_INJECTION_EXC = 28,
	IDT_VMM_COM_EXC = 29,
	IDT_SECURITY_EXC = 30,
	IDT_TIMER_INT = 60,
	IDT_KEYBOARD_INT = 61,
	IDT_SYSCALL_INT = 0x80,
	IDT_SPURIOUS_INT = 0xff
};

struct idt_entry {
	u16 offset0_15;
	u16 selector; // code segment
	u8  ist;
	u8  attributes;
	u16 offset16_31;
	u32 offset32_63;
	u32 zero;
} __attribute__((packed));

struct idtr {
	u16 limit;
	u64 base;
} __attribute__((packed));

void	idt_init(void);
void	idt_debug_call(u8 vec);
#endif