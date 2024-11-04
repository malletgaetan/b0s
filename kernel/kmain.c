#include "kernel/cpu.h"
#include "kernel/drivers/acpi/acpi.h"
#include "kernel/drivers/vga/vga.h" // NOOO
#include "kernel/interrupts.h"
#include "kernel/mm/kheap.h"
#include "kernel/mm/layout.h"
#include "kernel/mm/paging.h"
#include "kernel/mm/pmm.h"
#include "kernel/mm/vmm.h"
#include "kernel/multiboot2.h"
#include "kernel/multitasking/process.h"
#include "kernel/multitasking/sched.h"
#include "kernel/types.h"

#include "kernel/lib/debug/debug.h"
#include "kernel/lib/math/math.h"
#include "kernel/lib/printk/printk.h"
#include "kernel/lib/string/string.h"

const struct acpi_rsdp *rsdp = NULL;
const struct multiboot_tag_mmap *mmap = NULL;

static u8 read_tags(struct multiboot_tag *mb_infos)
{
	u64 size = *((u32 *)mb_infos);
	u32 bitmap = 0;
	u32 expected_bitmap = ((1 << MULTIBOOT_TAG_TYPE_ACPI_OLD) | (1 << MULTIBOOT_TAG_TYPE_MMAP));
	struct multiboot_tag *tag = (void *)mb_infos + 8;

	while (tag < mb_infos + (u64)size) {
		bitmap |= (1 << tag->type);
		switch (tag->type) {
		case MULTIBOOT_TAG_TYPE_ACPI_OLD:
			rsdp = (struct acpi_rsdp *)P2V(((struct multiboot_tag_old_acpi *)tag)->rsdp);
			break;
		case MULTIBOOT_TAG_TYPE_MMAP:
			mmap = (struct multiboot_tag_mmap *)P2V(tag);
			break;
		default:
			break;
		}
		if (tag->type == MULTIBOOT_TAG_TYPE_END)
			break;
		tag = (struct multiboot_tag *)((u64)tag + ((tag->size + 7) & ~7));
	}
	return ((bitmap & expected_bitmap) != expected_bitmap);
}

// idle task
void schleeeeep(void)
{
	while (TRUE) {
		sched_switch();
		cpu_halt();
	}
}

struct process *create_init_process(void)
{
	u64 program_entry = (u64)0x1000;
	struct process *init = process_create("init", program_entry, NULL);
	if (init == NULL)
		panic("%s: failed to create init process", __func__);
	if (vmm_alloc_at(init->space, (void *)program_entry, 1, PAGE_USER_RO) == NULL)
		panic("OHH");

	mmu_switch_space(init->space);

	*(u8 *)program_entry = 0xeb;
	*((u8 *)program_entry + 1) = 0xfe;

	return init;
}

int kmain(u32 magic, void *mb_infos_ptr)
{
	vga_reset(); // TODO: shouldn't rely on pure driver here, should abstract console

	// TODO: put mb_infos_ptr to virtual address space
	if (magic != MULTIBOOT2_BOOTLOADER_MAGIC)
		panic("only support bootloaders following the multiboot2 specifications");
	if ((u64)mb_infos_ptr & 7)
		panic("unaligned mb_infos_ptr");

	if (read_tags(mb_infos_ptr))
		panic("missing tags from bootloader");

	interrupts_entry_init();

	u64 available_pages = pmm_init(mmap);
	printk("kernel: %u MiB available\n", BYTE2MB(available_pages * PAGE_SIZE_IN_BYTES));
	vmm_init();
	kheap_init(1);

	acpi_init(rsdp);
	interrupts_init();

	process_init(); // set current
	// struct process *init = create_init_process();
	// sched_add(init);

	sched_start();
	schleeeeep();
	panic("%s: unreachable", __func__);
}
