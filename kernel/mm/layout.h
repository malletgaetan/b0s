#ifndef MEMORY_LAYOUT_H
# define MEMORY_LAYOUT_H

# define BIOS_END 0x100000
# define UEFI_END 0x200000

# define LOWER_HALF_START 0x0000000000000000
# define LOWER_HALF_STOP 0x00007fffffffffff

# define HIGHER_HALF_START 0xffff800000000000
# define HIGHER_HALF_STOP 0xffffffffffffffff

# define MAXIMUM_SUPPORTED_MEMORY_IN_BYTES 0x1000000000 // 64gb

# define KERNEL_VMA 0xffffffff80000000

# if !defined(ASM_FILE) && !defined(LINKER_FILE)
# include "kernel/types.h"
# include "kernel/multiboot2.h"

# define V2P(x) (((u8 *)x) - KERNEL_VMA)
# define P2V(x) (((u8 *)x) + KERNEL_VMA)

# define DM_V2P(x) (((u8 *)x) - HIGHER_HALF_START)
# define DM_P2V(x) (((u8 *)x) + HIGHER_HALF_START)

extern char kernel_vma_ro_start[]; // first valid address of kernel vma ro
extern char kernel_vma_ro_stop[]; // first invalid address of kernel vma ro

extern char kernel_vma_rw_start[]; // first valid address of kernel vma rw
extern char kernel_vma_rw_stop[]; // first invalid address of kernel vma rw

extern char kernel_vma_start[]; // first valid address of kernel va
extern char kernel_vma_stop[]; // first invalid address of kernel va

extern void	*pmm_vma_stop; // set in pmm.c

# endif
#endif