#include "kernel/arch/x86_64/interrupts/apic.h"

void sched_start(void) {
	local_apic_periodic_timer();
}