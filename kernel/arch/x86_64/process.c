#include "kernel/mm/vmm.h"


#include "kernel/arch/x86_64/process.h"
#include "kernel/arch/x86_64/cpu.h"
#include "kernel/arch/x86_64/mm/gdt.h"

#include "kernel/lib/string/string.h"
#include "kernel/lib/debug/debug.h"

extern u64 *process_switch_asm(struct context *cur, struct context *next); // in switch.asm
extern u64	trap_ret;

// its possible process_switch does not returns
void process_switch(struct process *cur, struct process *next) {
	process_switch_asm(cur->context, next->context);
}

void process_first_run(void) {
	printk("successfully switched to userspace proc kernel stack");
	return ; // goes to trap_ret
}

void process_arch_init(struct process *proc, u64 entry) {
	u8 *sp = (u8 *)proc->kstack + PROCESS_KERNEL_STACK_SIZE;

	sp -= sizeof(struct trap_frame);
	proc->tf = (struct trap_frame *)sp;
	memset(proc->tf, 0, sizeof(struct trap_frame));
	// rip;
	// cs;
	// rflags;
	// rsp;
	// ss;
	proc->tf->rip = entry; // userspace rip

	sp -= 8;
	*(u64 *)sp = trap_ret; // in idt.asm

	sp -= sizeof(struct context);
	proc->context = (struct context *)sp;
	memset(proc->context, 0, sizeof(struct context));
	proc->context->rip = (u64)process_first_run; // kernel space rip
}


// void process_arch_boot_init(struct process *proc) {

// }