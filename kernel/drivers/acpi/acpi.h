#ifndef ACPI_H
# define ACPI_H

#include "kernel/types.h"

# define ACPI_MADT_SIGNATURE "APIC"
# define ACPI_HPET_SIGNATURE "HPET"

struct acpi_rsdp {
	char	signature[8];
	u8		checksum;
	char	OEMID[6];
	u8		revision;
	u32		rsdt_address;
} __attribute__ ((packed));

struct acpi_header {
	char 	signature[4];
	u32 	length;
	u8 	  	revision;
	u8 	  	checksum;
	char 	OEMID[6];
	char 	oem_table_id[8];
	u32 	oem_revision;
	u32 	creator_id;
	u32 	creator_revision;
} __attribute__ ((packed));

struct acpi_rsdt {
	struct acpi_header 		h;
	u32 					sdts[];
} __attribute__ ((packed));

struct acpi_hpet {
	struct acpi_header		h;
	u8						hardware_rev_id;
	u8						info;
	u16						pci_vendor_id;
	u8						address_space_id;
	u8						register_bit_width;
	u8						register_bit_offset;
	u8						reserved1;
	u64						address;
	u8						hpet_number;
	u16						minimum_tick;
	u8 						page_protection;
} __attribute__ ((packed));

enum {
	ACPI_MADT_LAPIC = 0,
	ACPI_MADT_IO_APIC = 1,
	ACPI_MADT_IO_APIC_INTERRUPT_SOURCE_OVERRIDE = 2,
	ACPI_MADT_IO_APIC_NMI = 3,
	ACPI_MADT_LOCAL_APIC_NMI = 4,
	ACPI_MADT_LAPIC_ADDRESS_OVERRIDE = 5,
	ACPI_MADT_LX2APIC = 9
};

struct acpi_madt {
	struct acpi_header		h;
	u32						lapic;
	u32						flags;
	u8 						entries[];
} __attribute__ ((packed));

struct acpi_madt_entry {
	u8	type;
	u8	length;
	u8 	fields[];
} __attribute__ ((packed));

struct acpi_madt_io_apic {
	struct acpi_madt_entry e;
	u8	apic_id;
	u8	reserved;
	u32	address;
	u32	global_system_interrupt_base;
} __attribute__ ((packed));

struct acpi_madt_entry	*acpi_madt_find(u8 type);
u8 						acpi_copy_table(char *signature, void *dst);
void  					*acpi_find_table(char *signature);
void					acpi_init(const struct acpi_rsdp *);

#endif