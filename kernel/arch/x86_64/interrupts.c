#include "kernel/arch/x86_64/asm.h"
#include "kernel/arch/x86_64/apic.h"
#include "kernel/arch/x86_64/pic.h"
#include "kernel/arch/x86_64/hpet.h"
#include "kernel/arch/x86_64/idt.h"
#include "kernel/arch/x86_64/gdt.h"

#include "kernel/acpi.h"
#include "kernel/lib/debug/debug.h"
#include "kernel/lib/printk/printk.h"

// WHY 2 STAGES INIT?
// we need to cut down interrupts coming from PIC + setup IDT to get better debug info
// on paging. But at this stage, we don't want to start parse every ACPI as its possible they aren't mapped
// within the pages setup by boot.asm.

void interrupts_entry_init(void) {
	gdt_reload();
	local_apic_init();
	pic_disable();
	idt_init();
}

void interrupts_init(void) {
	hpet_init();
	local_apic_timer_calibrate();
	io_apic_init();
}