#include "kernel/arch/x86_64/asm.h"
#include "kernel/arch/x86_64/mm/paging.h"

#include "kernel/mm/layout.h"
#include "kernel/mm/pmm.h"
#include "kernel/mm/vmm.h"
#include "kernel/types.h"

#include "kernel/lib/debug/debug.h"
#include "kernel/lib/math/math.h"
#include "kernel/lib/string/string.h"

extern u64 PML4[512];

static u64 mmu_translate_4k(u64 *pml4e, void *va)
{
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

static u64 mmu_translate_1gb(u64 *pml4, void *va)
{
	u16 pml4_index = PML4E_INDEX(va);
	u16 pdpt_index = PDPT_INDEX(va);

	u64 *pdpt;

	if (!(pml4[pml4_index] & PAGE_PRESENT))
		return ~((u64)0);
	pdpt = (u64 *)PAGE_ADDR(pml4[pml4_index], PAGE_TABLE_SIZE_IN_BYTES);

	if (!(pdpt[pdpt_index] & PAGE_PRESENT))
		return ~((u64)0);
	return PAGE_ADDR(pdpt[pdpt_index], BOOT_PAGE_SIZE_IN_BYTES) |
		   ((u64)va & ((u64)BOOT_PAGE_SIZE_IN_BYTES - 1));
}

#ifdef DEBUG
// compare mapping from old vs new page tables on kernel code and data (pmm allocs inlcuded)
void mmu_test_kernel_mapping(struct vmm_space *space)
{
	for (u64 addr = (u64)kernel_vma_start; addr < (u64)pmm_vma_stop; addr += PAGE_SIZE_IN_BYTES) {
		u64 old = mmu_translate_1gb((u64 *)P2V(PML4), (void *)addr);
		u64 new = mmu_translate_4k(space->page_directory, (void *)addr);
		if (old != new) {
			panic("%s: addr=%p\nold=%p\nnew=%p\n", __func__, (void *)addr, (void *)old,
				  (void *)new);
		}
	}
}
#endif

void mmu_test_compare_mapping(struct vmm_space *space1, struct vmm_space *space2)
{
	struct vmm_region *region1 = space1->region;
	struct vmm_region *region2 = space2->region;

	while (region1 != NULL && region2 != NULL) {
		if (region1->va_start != region2->va_start)
			panic("%s: regions don't have the same va_start", __func__);
		if (region1->va_stop != region2->va_stop)
			panic("%s: regions don't have the same va_stop", __func__);
		if (region1->flags != region2->flags)
			panic("%s: regions don't have the same flags", __func__);

		for (u64 va = (u64)region1->va_start; va < (u64)region1->va_stop;
			 va += PAGE_SIZE_IN_BYTES) {
			u64 pa1 = mmu_translate_4k(space1->page_directory, (void *)va);
			u64 pa2 = mmu_translate_4k(space2->page_directory, (void *)va);
			if (pa1 != pa2)
				panic("%s: mismatch address", __func__);
		}

		region1 = region1->next;
		region2 = region2->next;
	}
	if (region1 != NULL || region2 != NULL)
		panic("%s: spaces don't have the same number of regions", __func__);
}

// pmm_alloc returns a pointer to physical address, that get translated to the direct mapping, as it
// will be present in init and running of the OS
u64 mmu_map_page(struct vmm_space *space, void *va, void *pa, u64 perm)
{
	ASSERT(IS_ALIGNED(va, PAGE_SIZE_IN_BYTES), "%s: va unaligned parameter %p\n", __func__, va);
	ASSERT(IS_ALIGNED(pa, PAGE_SIZE_IN_BYTES), "%s: pa unaligned parameter %p\n", __func__, pa);

	u64 perm_wo_g = perm & ~((u64)PAGE_GLOBAL);

	u64 ret;
	u16 pml4e_index = PML4E_INDEX(va);
	u16 pdpt_index = PDPT_INDEX(va);
	u16 pdt_index = PDT_INDEX(va);
	u16 pt_index = PT_INDEX(va);

	u64 *pml4e = (u64 *)space->page_directory;
	u64 *pdpt;
	u64 *pdt;
	u64 *pt;

	if (!(pml4e[pml4e_index] & PAGE_PRESENT)) {
		pdpt = pmm_xalloc();
		memset(DM_P2V(pdpt), 0, PAGE_SIZE_IN_BYTES);
		pml4e[pml4e_index] = (u64)pdpt | PAGE_PRESENT | perm_wo_g;
	}
	pdpt = (u64 *)(PAGE_ADDR(DM_P2V(pml4e[pml4e_index]), PAGE_TABLE_SIZE_IN_BYTES));

	if (!(pdpt[pdpt_index] & PAGE_PRESENT)) {
		pdt = pmm_xalloc();
		memset(DM_P2V(pdt), 0, PAGE_SIZE_IN_BYTES);
		pdpt[pdpt_index] = (u64)pdt | PAGE_PRESENT | perm_wo_g;
	}
	pdt = (u64 *)(PAGE_ADDR(DM_P2V(pdpt[pdpt_index]), PAGE_TABLE_SIZE_IN_BYTES));

	if (!(pdt[pdt_index] & PAGE_PRESENT)) {
		pt = pmm_xalloc();
		memset(DM_P2V(pt), 0, PAGE_SIZE_IN_BYTES);
		pdt[pdt_index] = (u64)pt | PAGE_PRESENT | perm_wo_g;
	}
	pt = (u64 *)(PAGE_ADDR(DM_P2V(pdt[pdt_index]), PAGE_TABLE_SIZE_IN_BYTES));
	ret = pt[pt_index];
	pt[pt_index] = (u64)pa | perm;
	return ret;
}

// NOTE: for the moment unmap will not be freeing unused page tables
u64 mmu_unmap_page(struct vmm_space *space, void *va)
{
	u64 pa = mmu_map_page(space, va, 0, 0);
	pmm_free((void *)PAGE_ADDR(pa, PAGE_SIZE_IN_BYTES));
	invlpg((u64)va);
	return pa;
}

void mmu_copy_pages(struct vmm_space *dst, struct vmm_space *src, struct vmm_region *region)
{
	u64 pa;
	for (u64 va = (u64)region->va_start; va < (u64)region->va_stop; va += PAGE_SIZE_IN_BYTES) {
		pa = mmu_translate_4k(src->page_directory, (void *)va);
		if (mmu_map_page(dst, (void *)va, (void *)pa, region->flags) != 0)
			panic("%s: overwrite preexisting mapping", __func__);
	}
}

void mmu_switch_space(struct vmm_space *space)
{
	paging_load((u64)DM_V2P(space->page_directory));
}

void mmu_init(struct vmm_space *space)
{
	space->page_directory = DM_P2V(pmm_xalloc());
	memset(space->page_directory, 0, PAGE_SIZE_IN_BYTES);
}
