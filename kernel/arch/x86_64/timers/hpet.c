#include "kernel/arch/x86_64/timers/hpet.h"
#include "kernel/arch/x86_64/asm.h"

#include "kernel/drivers/acpi/acpi.h"
#include "kernel/mm/layout.h"
#include "kernel/mm/paging.h"
#include "kernel/mm/vmm.h"

#include "kernel/lib/debug/debug.h"
#include "kernel/lib/math/math.h"
#include "kernel/lib/printk/printk.h"

struct acpi_hpet hpet;
u64 base;
u32 clock; // femto seconds

// HPET main counter will wrap around every 200 days

void hpet_sleep(u32 milliseconds)
{
	u64 target = volatile_read64((u64 *)(base + HPET_MAIN_COUNTER_VALUE)) +
				 (milliseconds * 1000000000000) / clock;
	while (volatile_read64((u64 *)(base + HPET_MAIN_COUNTER_VALUE)) < target)
		; // HPET count up
}

void hpet_init(void)
{
	if (acpi_copy_table(ACPI_HPET_SIGNATURE, &hpet))
		panic("%s: failed to retrieve hpet acpi table", __func__);

	if (!IS_ALIGNED(hpet.address, PAGE_SIZE_IN_BYTES))
		panic("%s: HPET address isn't aligned gl\n", __func__);

	base = (u64)vmm_map(kspace, (void *)DM_P2V(hpet.address), (void *)hpet.address, PAGE_KERNEL_RW);
	if (base == NULL)
		panic("%s: failed to map HPET", __func__);

	clock =
		volatile_read64((u64 *)(base + HPET_GENERAL_CAPABILITIES)) >> HPET_CAP_COUNTER_CLOCK_OFFSET;
	volatile_write64((u64 *)(base + HPET_GENERAL_CONFIGURATION), HPET_CONFIGURATION_OFF);
	volatile_write64((u64 *)(base + HPET_MAIN_COUNTER_VALUE), 0);
	volatile_write64((u64 *)(base + HPET_GENERAL_CONFIGURATION), HPET_CONFIGURATION_ON);
}