#ifndef ARCH_CPU_H
# define ARCH_CPU_H

#include "kernel/types.h"

struct cpu_status {
	u64 r15;
	u64 r14;
	u64 r13;
	u64 r12;
	u64 r11;
	u64 r10;
	u64 r9;
	u64 r8;
	u64 rdi;
	u64 rsi;
	u64 rbp;
	u64 rdx;
	u64 rcx;
	u64 rbx;
	u64 rax;

	u64 vector_number;
	u64 error_code;

	u64 rip;
	u64 cs;
	u64 rflags;
	u64 rsp;
	u64 ss;
} __attribute__((__packed__));

static inline void cpu_halt(void) {
	__asm__ volatile ("hlt");
}


#endif