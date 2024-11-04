#include "kernel/mm/vmm.h"

#include "kernel/arch/x86_64/cpu.h"
#include "kernel/arch/x86_64/mm/gdt.h"
#include "kernel/arch/x86_64/mm/paging.h"
#include "kernel/arch/x86_64/multitasking/process.h"

#include "kernel/lib/debug/debug.h"
#include "kernel/lib/string/string.h"

extern u64 *process_switch_asm(struct context **cur, struct context *next); // in switch.asm
extern void trap_ret(void);

// its possible process_switch does not returns
void process_switch(struct process *cur, struct process *next)
{
	if (cur == next)
		return;
	gdt_set_tss_rsp((u64)next->kstack + (u64)PROCESS_KERNEL_STACK_SIZE - (u64)1);
	mmu_switch_space(next->space);
	process_switch_asm(&cur->context, next->context);
}

void process_first_run(void)
{
	// for the moment we have nothing to put here, it'll come don't worry
	return; // goes to trap_ret
}

void process_arch_init(struct process *proc, u64 uentry, u64 ustack)
{
	u8 *sp = (u8 *)proc->kstack + PROCESS_KERNEL_STACK_SIZE;

	sp -= sizeof(struct trap_frame);
	proc->tf = (struct trap_frame *)sp;
	memset(proc->tf, 0, sizeof(struct trap_frame));
	proc->tf->rsp = ustack;
	proc->tf->ss = (GDT_USER_DATA_INDEX * 8) | DPL_USER;
	proc->tf->cs = (GDT_USER_CODE_INDEX * 8) | DPL_USER;
	proc->tf->rflags = 0x202;
	proc->tf->rip = uentry; // userspace rip

	sp -= 8;
	*(u64 *)sp = (u64)&trap_ret; // in idt.asm

	sp -= sizeof(struct context);
	proc->context = (struct context *)sp;
	memset(proc->context, 0, sizeof(struct context));
	proc->context->rip = (u64)process_first_run; // kernel space rip
}
