#ifndef ASM_H
#define ASM_H
#include "kernel/types.h"

#include "kernel/arch/x86_64/interrupts/idt.h"

// https://wiki.osdev.org/Inline_Assembly/Examples

static inline void load_tr(u16 selector)
{
	__asm__ __volatile__("ltr %0" : : "r"(selector) : "memory");
}

static inline void lidt(struct idtr *idtr)
{
	__asm__ __volatile__("lidt (%0)" : : "r"(idtr) : "memory");
}

static inline void cli(void)
{
	__asm__ __volatile__("cli" : : : "memory", "cc");
}

static inline void sti(void)
{
	__asm__ __volatile__("sti" : : : "memory", "cc");
}

// leaf -> eax | subleaf -> ecx
static inline void cpuid(u32 leaf, u32 subleaf, u32 *eax, u32 *ebx, u32 *ecx, u32 *edx)
{
	asm volatile("cpuid"
				 : "=a"(*eax), "=b"(*ebx), "=c"(*ecx), "=d"(*edx)
				 : "a"(leaf), "c"(subleaf));
}

static inline u8 inb(u16 port)
{
	u8 ret;
	__asm__ volatile("inb %w1, %b0" : "=a"(ret) : "Nd"(port) : "memory");
	return ret;
}

static inline void outb(u16 port, u8 val)
{
	__asm__ volatile("outb %b0, %w1" : : "a"(val), "Nd"(port) : "memory");
}

static inline void paging_load(u64 new_cr3)
{
	__asm__ volatile("mov %0, %%cr3\n" : : "r"(new_cr3) : "memory");
}

static inline u64 rdmsr(u32 msr)
{
	u32 low, high;
	__asm__ volatile("rdmsr" : "=a"(low), "=d"(high) : "c"(msr));
	return ((u64)high << 32) | low;
}

static inline void wrmsr(u32 msr, u64 value)
{
	u32 low = value & U32_MAX;
	u32 high = value >> 32;
	__asm__ volatile("wrmsr" : : "c"(msr), "a"(low), "d"(high));
}

static inline void invlpg(u64 addr)
{
	asm volatile("invlpg (%0)" : : "r"(addr) : "memory");
}

static inline u64 volatile_read64(u64 *addr)
{
	return *((volatile u64 *)addr);
}

static inline void volatile_write64(u64 *addr, u64 value)
{
	*((volatile u64 *)addr) = value;
}

static inline u32 volatile_read32(u32 *addr)
{
	return *((volatile u64 *)addr);
}

static inline void volatile_write32(u32 *addr, u32 value)
{
	*((volatile u32 *)addr) = value;
}

#endif