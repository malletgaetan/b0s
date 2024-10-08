#include "kernel/arch/x86_64/mm/gdt.h"
#include "kernel/arch/x86_64/asm.h"

#include "kernel/lib/printk/printk.h"

# define SEG_USERLAND (3 << 5)
# define SEG_KERNELLAND 0

#define GDT_ENTRY(a, b) ((struct gdt_entry){.flags=(a), .access=(GDT_ACCESS_PRESENT | (b))})

struct tss tss;
extern u64 stack_top_64;

struct gdt_entry gdt[GDT_SIZE] = {
	[GDT_NULL_INDEX] = {
		.limit0_15 = 0,
		.limit16_19 = 0,
		.base0_15 = 0,
		.base16_23 = 0,
		.base24_31 = 0,
		.flags = 0,
		.access = 0
	},
	[GDT_KERNEL_CODE_INDEX] = GDT_ENTRY(GDT_FLAGS_LONG_MODE, GDT_ACCESS_KERNEL | GDT_ACCESS_CODE_DATA_SEG | GDT_ACCESS_CODE | GDT_ACCESS_RW),
	[GDT_KERNEL_DATA_INDEX] = GDT_ENTRY(0, GDT_ACCESS_KERNEL | GDT_ACCESS_CODE_DATA_SEG | GDT_ACCESS_DATA | GDT_ACCESS_RW),
	[GDT_USER_CODE_INDEX] = GDT_ENTRY(GDT_FLAGS_LONG_MODE, GDT_ACCESS_USER | GDT_ACCESS_CODE_DATA_SEG | GDT_ACCESS_CODE | GDT_ACCESS_RW),
	[GDT_USER_DATA_INDEX] = GDT_ENTRY(0, GDT_ACCESS_USER | GDT_ACCESS_CODE_DATA_SEG | GDT_ACCESS_DATA | GDT_ACCESS_RW)
};

extern void	gdt_reload_asm(struct gdt_descriptor *desc);

void gdt_init_tss(void) {
	// set values of TSS
    tss.reserved0 = 0x00;
    tss.reserved1 = 0x00;
    tss.reserved2 = 0x00;
    tss.reserved3 = 0x00;
    tss.reserved4 = 0x00;

    tss.rsp0 = stack_top_64;
    tss.rsp1 = 0x0;
    tss.rsp2 = 0x0;

    tss.ist1 = 0x0;
    tss.ist2 = 0x0;
    tss.ist3 = 0x0;
    tss.ist4 = 0x0;
    tss.ist5 = 0x0;
    tss.ist6 = 0x0;
    tss.ist7 = 0x0;

    tss.io_bitmap_offset = 0x0;

	struct tss_entry *gdt_tss_entry = (struct tss_entry *)&gdt[GDT_TSS_INDEX];

	gdt_tss_entry->limit = sizeof(struct tss) - 1;
	gdt_tss_entry->base0_15 = (u16)((u64)(&tss) & U16_MAX);
	gdt_tss_entry->base16_23 = (u8)(((u64)(&tss) >> 16) & 0xff);
	gdt_tss_entry->base24_31 = (u8)(((u64)(&tss) >> 24) & 0xff);
	gdt_tss_entry->base32_63 = (u32)((u64)(&tss) >> 32);
	gdt_tss_entry->reserved = 0;
	gdt_tss_entry->access = GDT_ACCESS_KERNEL | GDT_ACCESS_PRESENT | GDT_ACCESS_SYSTEM_SEG | GDT_ACCESS_TSS;
	gdt_tss_entry->flags2 = 0x0;
	load_tr((u16)GDT_TSS_INDEX * 8);
}

void gdt_reload(void) {
	struct gdt_descriptor desc = (struct gdt_descriptor){
		.size=GDT_SIZE * 8,
		.offset=(u64)gdt,
	};
	gdt_reload_asm(&desc);
	gdt_init_tss();
}
