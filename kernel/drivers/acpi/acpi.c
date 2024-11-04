#include "kernel/drivers/acpi/acpi.h"
#include "kernel/mm/layout.h"

#include "kernel/lib/debug/debug.h"
#include "kernel/lib/printk/printk.h"
#include "kernel/lib/string/string.h"

static const struct acpi_rsdt *rsdt = NULL; // reside in direct mapping
static const struct acpi_madt *madt = NULL; // reside in direct mapping

static u8 checksum(u8 *ptr, u64 size)
{
	u8 sum = 0;
	while (size--)
		sum += ptr[size];
	return sum;
}

u8 acpi_copy_table(char *signature, void *dst)
{
	void *ptr = acpi_find_table(signature);
	if (ptr == NULL)
		return (1);
	memcpy(dst, ptr, (u64)(((struct acpi_header *)ptr)->length));
	return (0);
}

void *acpi_find_table(char *signature)
{
	ASSERT(rsdt, "%s: rsdt not initialized", __func__);
	u32 entries = (rsdt->h.length - sizeof(rsdt->h)) / 4;

	for (u32 i = 0; i < entries; i++) {
		struct acpi_header *h = (struct acpi_header *)DM_P2V(((u64)(rsdt->sdts[i])));
		if (!memcmp((u8 *)h->signature, (u8 *)signature, 4))
			return (void *)h;
	}

	return NULL;
}

void acpi_init(const struct acpi_rsdp *rsdp)
{
	if (memcmp((u8 *)rsdp->signature, (u8 *)"RSD PTR ", sizeof(rsdp->signature)) ||
		checksum((u8 *)rsdp, sizeof(struct acpi_rsdp)))
		panic("%s: invalid rsdp", __func__);

	rsdt = (struct acpi_rsdt *)DM_P2V((u64)rsdp->rsdt_address);

	if (checksum((u8 *)rsdt, (u64)rsdt->h.length))
		panic("%s: invalid rsdt", __func__);
}

struct acpi_madt_entry *acpi_madt_find(u8 type)
{
	if (madt == NULL) {
		madt = acpi_find_table(ACPI_MADT_SIGNATURE);
		if (checksum((u8 *)madt, (u64)madt->h.length))
			panic("%s: invalid madt", __func__);
	}

	ASSERT(madt, "%s: couldn't find acpi_madt", __func__);

	struct acpi_madt_entry *entry = (struct acpi_madt_entry *)madt->entries;
	u8 *end = (u8 *)madt + madt->h.length;
	while ((u8 *)entry < end) {
		if (entry->type == type)
			return entry;
		entry = (struct acpi_madt_entry *)((u8 *)entry + entry->length);
	}
	return NULL;
}