#include "kernel/arch/x86_64/mm/paging.h"
#include "kernel/arch/x86_64/asm.h"

#include "kernel/types.h"
#include "kernel/mm/vmm.h"
#include "kernel/mm/pmm.h"
#include "kernel/mm/layout.h"

#include "kernel/lib/math/math.h"
#include "kernel/lib/debug/debug.h"
#include "kernel/lib/string/string.h"

extern u64 	PML4[512];

static u64 mmu_translate_4k(u64 *pml4e, void *va) {
	ASSERT(IS_ALIGNED(va, PAGE_SIZE_IN_BYTES), "%s: va unaligned parameter %p\n", __func__, va);

	u16 pml4e_index = PML4E_INDEX(va);
	u16 pdpt_index = PDPT_INDEX(va);
	u16 pdt_index = PDT_INDEX(va);
	u16 pt_index = PT_INDEX(va);

	u64 *pdpt;
	u64 *pdt;
	u64 *pt;


	if (!(pml4e[pml4e_index] & PAGE_PRESENT))
		return ~((u64)0);
	pdpt = (u64 *)(PAGE_ADDR(DM_P2V(pml4e[pml4e_index]), PAGE_TABLE_SIZE_IN_BYTES));

	if (!(pdpt[pdpt_index] & PAGE_PRESENT))
		return ~((u64)0);
	pdt = (u64 *)(PAGE_ADDR(DM_P2V(pdpt[pdpt_index]), PAGE_TABLE_SIZE_IN_BYTES));

	if (!(pdt[pdt_index] & PAGE_PRESENT))
		return ~((u64)0);
	pt = (u64 *)(PAGE_ADDR(DM_P2V(pdt[pdt_index]), PAGE_TABLE_SIZE_IN_BYTES));

	return PAGE_ADDR(pt[pt_index], PAGE_SIZE_IN_BYTES) | ((u64)va & ((u64)PAGE_SIZE_IN_BYTES - 1));
}

static u64 mmu_translate_1gb(u64 *pml4, void *va) {
	u16 pml4_index = PML4E_INDEX(va);
	u16 pdpt_index = PDPT_INDEX(va);

	u64 *pdpt;

	if (!(pml4[pml4_index] & PAGE_PRESENT))
		return ~((u64)0);
	pdpt = (u64 *)PAGE_ADDR(pml4[pml4_index], PAGE_TABLE_SIZE_IN_BYTES);

	if (!(pdpt[pdpt_index] & PAGE_PRESENT))
		return ~((u64)0);
	return PAGE_ADDR(pdpt[pdpt_index], BOOT_PAGE_SIZE_IN_BYTES) | ((u64)va & ((u64)BOOT_PAGE_SIZE_IN_BYTES - 1));
}

// pmm_alloc returns a pointer to physical address, that get translated to the direct mapping, as it will be present in init and running of the OS
static u64 mmu_set_page(u64 *pml4e, void *va, void *pa, u64 perm) {
	ASSERT(IS_ALIGNED(va, PAGE_SIZE_IN_BYTES), "%s: va unaligned parameter %p\n", __func__, va);
	ASSERT(IS_ALIGNED(pa, PAGE_SIZE_IN_BYTES), "%s: pa unaligned parameter %p\n", __func__, pa);

	u64 ret;
	u16 pml4e_index = PML4E_INDEX(va);
	u16 pdpt_index = PDPT_INDEX(va);
	u16 pdt_index = PDT_INDEX(va);
	u16 pt_index = PT_INDEX(va);

	u64 *pdpt;
	u64 *pdt;
	u64 *pt;

	if (!(pml4e[pml4e_index] & PAGE_PRESENT)) {
		pdpt = pmm_xalloc();
		memset(DM_P2V(pdpt), 0, PAGE_SIZE_IN_BYTES);
		pml4e[pml4e_index] = (u64)pdpt | PAGE_PRESENT | perm;
	}
	pdpt = (u64 *)(PAGE_ADDR(DM_P2V(pml4e[pml4e_index]), PAGE_TABLE_SIZE_IN_BYTES));

	if (!(pdpt[pdpt_index] & PAGE_PRESENT)) {
		pdt = pmm_xalloc();
		memset(DM_P2V(pdt), 0, PAGE_SIZE_IN_BYTES);
		pdpt[pdpt_index] = (u64)pdt | PAGE_PRESENT | perm;
	}
	pdt = (u64 *)(PAGE_ADDR(DM_P2V(pdpt[pdpt_index]), PAGE_TABLE_SIZE_IN_BYTES));

	if (!(pdt[pdt_index] & PAGE_PRESENT)) {
		pt = pmm_xalloc();
		memset(DM_P2V(pt), 0, PAGE_SIZE_IN_BYTES);
		pdt[pdt_index] = (u64)pt | PAGE_PRESENT | perm;
	}
	pt = (u64 *)(PAGE_ADDR(DM_P2V(pdt[pdt_index]), PAGE_TABLE_SIZE_IN_BYTES));
	ret = pt[pt_index];
	pt[pt_index] = (u64)pa | perm;
	return ret;
}

u64 mmu_map_page(struct vmm_space *space, void *va, void *pa, u64 flags) {
	return mmu_set_page(space->page_directory, va, pa, flags);
}

// NOTE: for the moment unmap will not be freeing unused page tables
u64 mmu_unmap_page(struct vmm_space *space, void *va) {
	u64 pa = mmu_set_page(space->page_directory, va, 0, 0);
	pmm_free((void *)PAGE_ADDR(pa, PAGE_SIZE_IN_BYTES));
	invlpg((u64)va);
	return pa;
}

void mmu_switch_space(struct vmm_space *space) {
	paging_load((u64)DM_V2P(space->page_directory));
}

void mmu_init(struct vmm_space *space) {
	space->page_directory = DM_P2V(pmm_xalloc());
	memset(space->page_directory, 0, PAGE_SIZE_IN_BYTES);
}

// compare mapping from old vs new page tables on kernel code and data (pmm allocs inlcuded)
void mmu_test_kernel_mapping(struct vmm_space *space) {
	for (u64 addr = (u64)kernel_vma_start; addr < (u64)pmm_vma_stop; addr += PAGE_SIZE_IN_BYTES) {
		u64 old = mmu_translate_1gb((u64 *)P2V(PML4), (void *)addr);
		u64 new = mmu_translate_4k(space->page_directory, (void *)addr);
		if (old != new) {
			panic("%s: addr=%p\nold=%p\nnew=%p\n", __func__, (void *)addr, (void *)old, (void *)new);
		}
	}
}
