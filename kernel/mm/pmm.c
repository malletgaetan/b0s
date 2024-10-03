#include "kernel/mm/pmm.h"
#include "kernel/mm/layout.h"
#include "kernel/mm/paging.h"

#include "kernel/lib/math/math.h"
#include "kernel/lib/string/string.h"
#include "kernel/lib/debug/debug.h"

void *pmm_vma_stop = NULL;

// NOTES:
// could be implemented as a big stack that contains each free page, was kinda
// scared I would need some type of long contiguous page allocations so gone for the simple
// and most flexible solution, but performance could be a lot better with a bit of optimizations or a buddy allocator

// pmap should be sorted by pmap[i].addr
static struct pmm_region pmap[PMAP_SIZE] = {0};
static u32 pmap_size = 0;
static char *type_debug[] = {
	[PMM_REGION_AVAILABLE] = "available",
	[PMM_REGION_RESERVED] = "reserved"
};

#ifdef DEBUG
static u8 pmap_sorted(void) {
	for (u32 i = 1; i < pmap_size; i++) {
		if ((u64)pmap[i].addr <= (u64)pmap[i - 1].addr)
			return 0;
	}
	return 1;
}
#endif

static void pmap_sort(void) {
	if (pmap_size == 0)
		return ;

	struct pmm_region tmp;
	for (u32 range = pmap_size; range > 1; range--) {
		u32 start = pmap_size - range;
		struct pmm_region *min = &pmap[start];
		for (u32 j = start + 1; j < pmap_size; j++ ) {
			if (min->addr > pmap[j].addr)
				min = pmap + j;
		}
		tmp = pmap[start];
		pmap[start] = *min;
		*min = tmp;
	}
}

static void pmap_insert(const struct multiboot_mmap_entry *entry, u32 pmap_index) {
	pmap[pmap_index].type = entry->type == MULTIBOOT_MEMORY_AVAILABLE ? PMM_REGION_AVAILABLE : PMM_REGION_RESERVED;

	if (pmap[pmap_index].type == PMM_REGION_AVAILABLE)
		pmap[pmap_index].addr = (void *)PAGE_ADDR(MAX(V2P(kernel_vma_stop), entry->addr), PAGE_SIZE_IN_BYTES); // TODO: shouldn't use a MAX here
	else
		pmap[pmap_index].addr = (void *)PAGE_ADDR((u64)entry->addr, PAGE_SIZE_IN_BYTES);

	u64 len = (entry->len - (u64)(pmap[pmap_index].addr - entry->addr));
	void *addr_stop = (void *)ROUNDUP(((u64)pmap[pmap_index].addr + len) - 1, PAGE_SIZE_IN_BYTES);
	pmap[pmap_index].size_in_pages = ((u64)addr_stop - (u64)pmap[pmap_index].addr) / PAGE_SIZE_IN_BYTES;

	for (u32 i = 0; i < pmap_index; i++) { // check for possible overlap
		void *_start = pmap[i].addr;
		void *_stop = (void *)((u64)pmap[i].addr + (u64)(pmap[i].size_in_pages * PAGE_SIZE_IN_BYTES));
		if (pmap[pmap_index].addr >= _stop || addr_stop <= _start)
			continue ;
		panic("%s: physical memory overlap", __func__);
	}
}

static void pmap_init(const struct multiboot_tag_mmap *mmap) {
	u32 nb_entries = (mmap->size - sizeof(struct multiboot_tag_mmap)) / mmap->entry_size;
	u32 pmap_index = 0;

	TRACE("%s: mmap:", __func__);
	for (u32 i = 0; i < nb_entries; i++) {
		TRACE("%p -> %p\n", mmap->entries[i].addr, mmap->entries[i].addr + mmap->entries[i].len);
		u8 is_available_or_reserved = ((mmap->entries[i].type == MULTIBOOT_MEMORY_AVAILABLE) && (mmap->entries[i].addr >= BIOS_END)) || (mmap->entries[i].type == MULTIBOOT_MEMORY_RESERVED);
		if (!is_available_or_reserved)
			continue ;

		if (pmap_index >= PMAP_SIZE || pmap_index == 0xff)
			panic("%s: statically allocated pmap not big enough\n");
		pmap_insert(&mmap->entries[i], pmap_index);
		pmap_index++;
	}
	pmap_size = pmap_index;
	pmap_sort();
	ASSERT(pmap_sorted(), "%s: pmap not sorted\n", __func__);
}

void pmm_dump_pmap(void) {
	for (u64 i = 0; i < pmap_size; i++)
		printk("pa_start=%p | pa_stop=%p ~%uKiB | type=%s\n", pmap[i].addr, (void *)((u64)pmap[i].addr + (pmap[i].size_in_pages * PAGE_SIZE_IN_BYTES)), BYTE2KB(pmap[i].size_in_pages * PAGE_SIZE_IN_BYTES), type_debug[pmap[i].type]);
}

struct pmm_phys_addr pmm_next_phys_page(void *addr) {
	ASSERT(IS_ALIGNED(addr, PAGE_SIZE_IN_BYTES), "%s: addr isn't aligned\n", __func__);

	u64 next = (u64)addr + PAGE_SIZE_IN_BYTES;
	for (u32 i = 0; i < pmap_size; i++) {
		u64 stop = (u64)pmap[i].addr + (pmap[i].size_in_pages * PAGE_SIZE_IN_BYTES);
		if (next >= stop)
			continue ;
		return (struct pmm_phys_addr){.pa=(void *)(MAX(next, (u64)pmap[i].addr)), .type=pmap[i].type};
	}
	return (struct pmm_phys_addr){.pa=(void *)1,.type=0}; // as 1 can't be addr of a page
}

u64 pmm_init(const struct multiboot_tag_mmap *mmap) {
	u64 available_pages = 0;
	u64 *bitmap_alloc = (u64 *)kernel_vma_stop;

	// retrieve memory information
	pmap_init(mmap); 

	// create bitmaps
	for (u32 i = 0; i < pmap_size; i++) {
		if (pmap[i].type != PMM_REGION_AVAILABLE)
			continue ;
		bitmap_alloc = bitmap_init(&pmap[i].optional.bitmap, pmap[i].size_in_pages, bitmap_alloc);
		available_pages += pmap[i].size_in_pages;
	}

	// integrate bitmaps to kernel mapping
	pmm_vma_stop = (void *)(ROUNDUP(bitmap_alloc - 1, PAGE_SIZE_IN_BYTES));
	for (u64 pa = (u64)V2P(kernel_vma_stop); pa < (u64)V2P(pmm_vma_stop); pa += PAGE_SIZE_IN_BYTES) {
		(void)pmm_alloc(); // TODO: could be dangerous if we change the pmm_alloc implementation, which currently return first free page
		--available_pages;
	}
	return available_pages;
}

void *pmm_xalloc(void) {
	void *ret = pmm_alloc();
	if (ret == NULL)
		panic("%s: no more memory", __func__);
	return ret;
}

void *pmm_alloc(void) {
	for (u32 i = 0; i < pmap_size; i++) {
		if (pmap[i].type != PMM_REGION_AVAILABLE)
			continue ;

		u64 j = bitmap_find_and_set(&pmap[i].optional.bitmap);
		if (j == pmap[i].optional.bitmap.len)
			continue ;
		return (void *)((u64)pmap[i].addr + (j * PAGE_SIZE_IN_BYTES));
	}
	return NULL; // let caller handle the situation
}

void pmm_free(void *ptr) {
	if ((u64)ptr <= (u64)V2P(pmm_vma_stop))
		panic("%s: freeing non freeable memory pages %p", __func__, ptr);

	for (u32 i = 0; i < pmap_size; i++) {
		if (pmap[i].type != PMM_REGION_AVAILABLE)
			continue ;

		u64 stop = (u64)pmap[i].addr + (pmap[i].size_in_pages * PAGE_SIZE_IN_BYTES);
		if ((u64)ptr >= stop || (u64)ptr < (u64)pmap[i].addr)
			continue ;
		u64 j = (u64)(ptr - pmap[i].addr) / PAGE_SIZE_IN_BYTES;
		return bitmap_unset(&pmap[i].optional.bitmap, j);
	}

	panic("%s: bad page free %p", __func__, ptr);
}

