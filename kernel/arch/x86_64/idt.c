#include "kernel/arch/x86_64/asm.h"
#include "kernel/arch/x86_64/apic.h"
#include "kernel/arch/x86_64/idt.h"
#include "kernel/arch/x86_64/gdt.h"
#include "kernel/lib/debug/debug.h"
#include "kernel/lib/printk/printk.h"

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
		case IDT_TIMER_INT: // OMG ITS WORKING
			count++;
			if (count == 100) {
				count = 0;
				printk("1 seconds passed\n");
			}
			local_apic_timer_arm();
			break ;
		case IDT_PAGE_FAULT: // page fault
			printk("PAGE FAULT:");
			printk("error occured at %p\n", status->rip);
			printk("error code %b\n", status->error_code);
			panic("look at at the CR2 reigister value to found out which address caused the page fault");
			break ;
		case IDT_SPURIOUS_INT:
			break ;
		default:
			printk("unexepected interrupt: vector %u\n", status->vector_number);
			printk("RIP %p\n", status->rip);
			panic("unexepected interrupt: error code %b\n", status->error_code);
			break ;
	}
	local_apic_eoi();
	return rsp;
}

void idt_set_entry(u8 vector, u64 handler, u8 type) {
	idt[vector].offset0_15 = handler & 0xffff;
	idt[vector].selector = GDT_KERNEL_CODE_INDEX * 8;
	idt[vector].ist = 0;
	idt[vector].attributes = type;
	idt[vector].offset16_31 = (handler >> 16) & 0xffff;
	idt[vector].offset32_63 = (handler >> 32) & 0xffffffff;
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
