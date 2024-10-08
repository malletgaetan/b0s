#include "kernel/arch/x86_64/cpu.h"
#include "kernel/arch/x86_64/asm.h"
#include "kernel/arch/x86_64/interrupts/apic.h"
#include "kernel/arch/x86_64/interrupts/idt.h"
#include "kernel/arch/x86_64/mm/gdt.h"
#include "kernel/lib/debug/debug.h"
#include "kernel/lib/printk/printk.h"

#include "kernel/task.h"
#include "kernel/sched.h"

extern u64 idt_vectors[];

__attribute__((aligned(0x10)))
struct idt_entry idt[IDT_SIZE];

void idt_debug_call(u8 vec) {
	((void (*)(void))idt_vectors[vec])();
}
u64 count = 0;

u64 interrupt_handler(u64 rsp) {
	struct cpu_status *status = (struct cpu_status *)rsp;

	switch (status->vector_number) {
		case IDT_TIMER_INT:
			// struct task *task = sched_run(status); // return the task to be ran
			// local_apic_timer_arm();
			// local_apic_eoi();
			// return (u64)task->context;
			break ;
		case IDT_PAGE_FAULT: // page fault
			// TODO: if userspace task -> kill else panic
			printk("PAGE FAULT:");
			printk("error occured at %p\n", status->rip);
			printk("error code %b\n", status->error_code);
			panic("look at at the CR2 reigister value to found out which address caused the page fault");
			break ;
		case IDT_SPURIOUS_INT:
			break ;
		default:
			panic("unexepected interrupt: vector %u | error %u\n", status->vector_number, status->error_code);
	}
	local_apic_eoi();
	return rsp;
}

void idt_set_entry(u8 vector, u64 handler, u8 type) {
	idt[vector].offset0_15 = handler & U16_MAX;
	idt[vector].selector = GDT_KERNEL_CODE_INDEX * 8;
	idt[vector].ist = 0;
	idt[vector].attributes = type;
	idt[vector].offset16_31 = (handler >> 16) & U16_MAX;
	idt[vector].offset32_63 = (handler >> 32) & U32_MAX;
	idt[vector].zero = 0;
}

void idt_init(void) {
	struct idtr idtr;
	idtr.limit = sizeof(idt) - 1;
	idtr.base = (u64)idt;

	for (int i = 0; i < IDT_SIZE; i++) {
		idt_set_entry(i, (u64)(idt_vectors[i]), IDT_GATE);
	}

	lidt(&idtr);
	sti();
}