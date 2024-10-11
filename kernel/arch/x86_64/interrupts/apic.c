#include "kernel/arch/x86_64/asm.h"
#include "kernel/arch/x86_64/timers/hpet.h"
#include "kernel/arch/x86_64/interrupts/idt.h"
#include "kernel/arch/x86_64/interrupts/apic.h"
#include "kernel/arch/x86_64/interrupts/pic.h"

#include "kernel/drivers/acpi/acpi.h"
#include "kernel/mm/vmm.h"
#include "kernel/mm/paging.h"
#include "kernel/mm/layout.h"

#include "kernel/lib/debug/debug.h"

u64 io_apic_base = 0;
u32 timer_ticks_per_10ms;

void io_apic_write_register(u8 offset, u32 val) {
	ASSERT(io_apic_base, "%s: io_apic isn't setup\n", __func__);

	volatile_write32((u32 *)io_apic_base, (u32)offset); // tell IO_APIC where we want to write
	volatile_write32((u32 *)(io_apic_base + (u64)IO_APIC_REGISTER_DATA), val); // what we want to write
}

u32 io_apic_read_register(u8 offset) {
	ASSERT(io_apic_base, "%s: io_apic isn't setup\n", __func__);

	volatile_write32((u32 *)io_apic_base, (u32)offset); // tell IO_APIC where we want to read
	return volatile_read32((u32 *)(io_apic_base + (u64)IO_APIC_REGISTER_DATA)); // read
}

void io_apic_init(void) {
	struct acpi_madt_io_apic *io_apic = (struct acpi_madt_io_apic *)acpi_madt_find(ACPI_MADT_IO_APIC);
	if (io_apic == NULL)
		panic("%s: couldn't find IO APIC in acpi_madt", __func__);

	io_apic_base = (u64)vmm_map(kspace, (void *)DM_P2V((u64)io_apic->address), (void *)(u64)io_apic->address, PAGE_KERNEL_RW);
	if (io_apic_base == NULL)
		panic("%s: failed to map IO APIC", __func__);
}

void local_apic_eoi(void) {
	wrmsr(EOI_REGISTER, 0);
}

// trigger an interrupt in 10 milliseconds
void local_apic_periodic_timer(void) {
	// TODO: handle multiple quanta
	// ->function takes parameter + set higher divider depending on millisecond value
	u32 reg = (rdmsr(LVT_TIMER_REGISTER) & ~(LVT_MASK)) | LVT_PERIODIC | IDT_TIMER_INT; // mask off + vector 100
	wrmsr(LVT_TIMER_REGISTER, reg);

	wrmsr(INITIAL_COUNT_REGISTER, timer_ticks_per_10ms); // will now wrap around every 10ms
}

// AMD64 volume2 16.4.1 APIC Timer Interrupt
void local_apic_timer_calibrate(void) {
	u32 conf = rdmsr(DIVIDE_CONFIGURATION_REGISTER);
	wrmsr(DIVIDE_CONFIGURATION_REGISTER, conf | TIMER_DIVIDE_BY_2); // divide frequency so we don't wrap around, even divide by 1 is sufficient on my current computer but let's play it safe
	wrmsr(INITIAL_COUNT_REGISTER, U32_MAX); // LAPIC timer count down

	hpet_sleep(10);

	timer_ticks_per_10ms = U32_MAX - rdmsr(CURRENT_COUNTER_REGISTER);

	wrmsr(INITIAL_COUNT_REGISTER, 0); // disable
}

void local_apic_init(void) {
	u32 eax, ebx, ecx, edx;
	cpuid(1, 0, &eax, &ebx, &ecx, &edx);
	if (!(edx & (1 << 9))) // TODO: create clean constants for these
		panic("%s: hardware is missing APIC", __func__);
	if (!(ecx & (1 << 21)))
		panic("%s: hardware is missing x2APIC", __func__);

	u64 flags = rdmsr(IA32_APIC_BASE_MSR);
	flags = flags | (1 << 10) | (1 << 11); // Intel Volume 3A: 10.12.1 Detecting and Enabling x2APIC Mode
	wrmsr(IA32_APIC_BASE_MSR, flags);

	wrmsr(SPURIOUS_INTERRUPT_VECTOR_REGISTER, (1 << 8) | IDT_SPURIOUS_INT);
}