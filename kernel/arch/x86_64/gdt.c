#include "kernel/arch/x86_64/gdt.h"

# define SEG_USERLAND (3 << 5)
# define SEG_KERNELLAND 0

#define GDT_ENTRY(a, b) ((struct gdt_entry){.flags=(a), .access=(GDT_ACCESS_PRESENT | (b))})

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
	[GDT_KERNEL_CODE_INDEX] = GDT_ENTRY(GDT_FLAGS_LONG_MODE, GDT_ACCESS_KERNEL | GDT_ACCESS_CODE | GDT_ACCESS_RW),
	[GDT_KERNEL_DATA_INDEX] = GDT_ENTRY(0, GDT_ACCESS_KERNEL | GDT_ACCESS_DATA | GDT_ACCESS_RW),
	[GDT_USER_CODE_INDEX] = GDT_ENTRY(GDT_FLAGS_LONG_MODE, GDT_ACCESS_USER | GDT_ACCESS_CODE | GDT_ACCESS_RW),
	[GDT_USER_DATA_INDEX] = GDT_ENTRY(0, GDT_ACCESS_USER | GDT_ACCESS_DATA | GDT_ACCESS_RW)
};

extern void	gdt_reload_asm(struct gdt_descriptor *desc);

void gdt_reload(void) {
	struct gdt_descriptor desc = (struct gdt_descriptor){
		.size=GDT_SIZE * 8,
		.offset=(u64)gdt,
	};
	gdt_reload_asm(&desc);
}