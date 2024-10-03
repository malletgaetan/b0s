#define LINKER_FILE
#include "kernel/mm/layout.h"
#include "kernel/mm/paging.h"

ENTRY(_entry_32)

SECTIONS
{
	PROVIDE(kernel_vma_start = KERNEL_VMA);
    PROVIDE(kernel_vma_rw_start = KERNEL_VMA);
    . = UEFI_END;
    .multiboot ALIGN(8) : AT(ADDR(.multiboot))
    {
        *(.multiboot)
    }

    /* TODO: since this code will not be reused, we should put it in a different page alignment and add it to the PMM */
    .entry_32 ALIGN(8): AT(ADDR(.entry_32))
    {
        *(.entry_32)
        *(.entry_64)
    }

    . += KERNEL_VMA;
    .data ALIGN(8) : AT(ADDR(.data) - KERNEL_VMA)
    {
        *(.data)
    }

    .bss ALIGN(8) : AT(ADDR(.bss) - KERNEL_VMA)
    {
        *(COMMON)
        *(.bss)
    }
    . = ALIGN(PAGE_SIZE_IN_BYTES);
    PROVIDE(kernel_vma_rw_stop = .);
    PROVIDE(kernel_vma_ro_start = .);

    .text ALIGN(8) : AT(ADDR(.text) - KERNEL_VMA)
    {
        *(.text)
    }

    .rodata ALIGN(8) : AT(ADDR(.rodata) - KERNEL_VMA)
    {
        *(.rodata)
    }
    . = ALIGN(PAGE_SIZE_IN_BYTES);
    PROVIDE(kernel_vma_ro_stop = .);
	PROVIDE(kernel_vma_stop = .);
}
