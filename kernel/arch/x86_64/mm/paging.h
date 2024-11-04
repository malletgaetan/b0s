#ifndef ARCH_PAGING_H
#define ARCH_PAGING_H

#define BOOT_PAGE_FLAG 0b10000000 // flag for 1gib
#define BOOT_PAGE_SIZE_IN_BYTES 0x40000000

#define PAGE_PRESENT 0b1
#define PAGE_RO 0b0
#define PAGE_RW 0b10
#define PAGE_KERNEL_RING 0b0
#define PAGE_USER_RING 0b100
#define PAGE_GLOBAL 0b100000000
// # define PAGE_PDE_PAGE_FLAG 0b10000000 // 2mb paging flag [also need CR4.PAE]
#define PAGE_TABLE_SIZE_IN_BYTES 4096 // 4kib
#define PAGE_SIZE_IN_BYTES 4096		  // 4kib

#if !defined(ASM_FILE) && !defined(LINKER_FILE)
#include "kernel/types.h"

#define PAGE_USER_RW (PAGE_PRESENT | PAGE_RW | PAGE_USER_RING)
#define PAGE_USER_RO (PAGE_PRESENT | PAGE_RO | PAGE_USER_RING)

#define DPL_KERN 0x0
#define DPL_USER 0x3

// # define PAGE_KERNEL_RW (PAGE_PRESENT | PAGE_RW | PAGE_KERNEL_RING)
// # define PAGE_KERNEL_RO (PAGE_PRESENT | PAGE_RO | PAGE_KERNEL_RING)

#define PAGE_KERNEL_RW (PAGE_PRESENT | PAGE_RW | PAGE_KERNEL_RING | PAGE_GLOBAL)
#define PAGE_KERNEL_RO (PAGE_PRESENT | PAGE_RO | PAGE_KERNEL_RING | PAGE_GLOBAL)

#define PAGE_ADDR(addr, page_size) (((u64)addr) & ~(page_size - 1))
#define PML4E_INDEX(x) ((u16)((((u64)x) >> 39) & 0x1ff))
#define PDPT_INDEX(x) ((u16)((((u64)x) >> 30) & 0x1ff))
#define PDT_INDEX(x) ((u16)((((u64)x) >> 21) & 0x1ff))
#define PT_INDEX(x) ((u16)((((u64)x) >> 12) & 0x1ff))

#endif

#endif