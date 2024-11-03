#ifndef TESTER
#include "kernel/mm/vmm.h"
#include "kernel/mm/layout.h"
#include "kernel/mm/paging.h"
#include "kernel/mm/pmm.h"

#include "kernel/lib/printk/printk.h"
#include "kernel/lib/math/math.h"
#include "kernel/lib/debug/debug.h"
#include "kernel/lib/bitmap/bitmap.h"
#endif

void *kernel_dm_start; // set in vmm.c
void *kernel_dm_stop; // set in vmm.c

enum struct_type {
	TYPE_VMM_REGION,
	TYPE_VMM_SPACE,
	NUM_TYPES
};

struct block *block_heads[NUM_TYPES] = {NULL};

struct vmm_space *kspace;

// START Virtual Memory Manager Object Allocator ------

// For the moment its implemented as a bitmap, could be a stack based allocator for fast
// alloc and dealloc but since it resides in the direct mapping, any overflow would be
// dramatic, for the moment we'll stay with simple solution will low chance of overflow.

struct block {
	void			*objects;
	u64 			nb_used;
	u64 			object_size_in_bytes;
	u64 			max_objects;
	struct bitmap	bm;
	struct block 	*prev;
	struct block 	*next;
};

static u64 get_struct_size(enum struct_type type) {
	switch (type) {
		case TYPE_VMM_REGION:
			return sizeof(struct vmm_region);
		case TYPE_VMM_SPACE:
			return sizeof(struct vmm_space);
		default:
			panic("%s: unknown type", __func__);
			return 0;
	}
}

static struct block *create_block(enum struct_type type, struct block *prev) {
	struct block *new_block = (struct block *)DM_P2V(pmm_xalloc());

	new_block->object_size_in_bytes = get_struct_size(type);
	new_block->max_objects = (PAGE_SIZE_IN_BYTES - sizeof(struct block)) / (new_block->object_size_in_bytes + sizeof(u8));
	new_block->objects = (void *)((u64)new_block + sizeof(struct block));
	new_block->prev = prev;
	new_block->next = NULL;
	void *last_used_map = bitmap_init(
		&new_block->bm,
		new_block->max_objects,
		(void *)((u64)new_block->objects + (new_block->max_objects * new_block->object_size_in_bytes))
	);
	(void)last_used_map;

	ASSERT((u64)last_used_map < ((u64)new_block + PAGE_SIZE_IN_BYTES), "%s: block overflow in direct mapping", __func__);

	return new_block;
}

static void *block_alloc(enum struct_type type) {
	if (block_heads[type] == NULL)
		block_heads[type] = create_block(type, NULL);

	struct block *current = block_heads[type];
	while (current != NULL) {
		u64 index = bitmap_find_and_set(&current->bm);
		if (index != current->bm.len) {
			current->nb_used++;
			return (void *)(current->objects + (index * current->object_size_in_bytes));
		}

		if (current->next == NULL)
			current->next = create_block(type, current);
		current = current->next;
	}

	return NULL;
}

static void block_free(void *ptr, enum struct_type type) {
	struct block *current = block_heads[type];
	while (current != NULL) {
		if ((u64)ptr < (u64)current->objects || (u64)ptr > ((u64)current->objects + (current->max_objects * current->object_size_in_bytes))) {
			current = current->next;
			continue ;
		}
		bitmap_unset(&current->bm, ((u64)ptr - (u64)current->objects) / current->object_size_in_bytes);
		current->nb_used--;
		if (current->nb_used == 0) {
			if (current->prev != NULL)
				current->prev->next = current->next;
			if (current->next != NULL)
				current->next->prev = current->prev;
			pmm_free(DM_V2P(current));
		}
		return ;
	}
	panic("%s: freeing unknown pointer", __func__);
}

// STOP  Virtual Memory Manager Object Allocator ------

#ifdef DEBUG
static void vmm_probe_space(struct vmm_space *space) {
	for (struct vmm_region *region = space->region; region != NULL; region = region->next) {
		if (region->flags == PAGE_KERNEL_RO || region->flags == PAGE_USER_RO)
			continue ;
		u8 useless = 0;
		for (u64 addr = (u64)region->va_start; addr < (u64)region->va_stop; addr += PAGE_SIZE_IN_BYTES)
			useless += *((u8 *)addr);
		(void)useless;
	}
}
#endif

static struct vmm_region *vmm_create_region(struct vmm_space *space, void *va_start, void *va_stop, u64 flags, struct vmm_region *prev, struct vmm_region *next) {
	struct vmm_region *region = block_alloc(TYPE_VMM_REGION);

	if (prev == NULL)
		space->region = region;

	region->va_start = va_start;
	region->va_stop = va_stop;
	region->flags = flags;
	region->next = next;
	region->prev = prev;

	if (region->prev != NULL)
		region->prev->next = region;
	if (region->next != NULL)
		region->next->prev = region;

	return region;
}

static void vmm_remove_region(struct vmm_space *space, struct vmm_region *region) {
	if (region->prev == NULL && region->next == NULL) {
		space->region = NULL;
	} else {
		if (space->region == region)
			space->region = region->next;
		if (region->prev != NULL)
			region->prev->next = region->next;
		if (region->next != NULL)
			region->next->prev = region->prev;
	}
	block_free(region, TYPE_VMM_REGION);
}

// START Virtual Memory Manager -----------------------
// vmm_create_region_between(...)
// - fail on regions overlaps
// - will merge regions if possible
// its important that regions that could seem 'mergeable' will not be merged.
// for example:
// --||||||-----
// ------||||---
// could result to:
// --||||||||---
// But by not yielding errors, the caller have no possibilities to know
// he did bad, and will then use the memory as if it was new, possibly overwriting preexisting data.
// one of next and prev must be present
struct vmm_region *vmm_insert_range(struct vmm_space *space, struct vmm_region *prev, struct vmm_region *next, void *va_start, void *va_stop, u64 flags) {
	if (next != NULL && va_stop > next->va_start)
		panic("%s:overlap|%p > %p", __func__, va_stop, next->va_start);
	if (prev != NULL && va_start < prev->va_stop)
		panic("%s:overlap|%p < %p", __func__, va_start, prev->va_stop);

	u8 connected_with_prev = prev != NULL && va_start == prev->va_stop && flags == prev->flags;
	u8 connected_with_next = next != NULL && va_stop == next->va_start && next->flags == flags;

	if (connected_with_prev && connected_with_next) {
		prev->va_stop = next->va_stop;
		vmm_remove_region(space, next);
		return prev;
	}

	if (connected_with_prev) {
		prev->va_stop = va_stop;
		return prev;
	}

	if (connected_with_next) {
		next->va_start = va_start;
		return next;
	}

	// create a new block in case no merging possible
	return vmm_create_region(space, va_start, va_stop, flags, prev, next);
}

// returns the region containing the va_start and va_stop
static struct vmm_region *vmm_add_range(struct vmm_space *space, void *va_start, void *va_stop, u64 flags) {
	ASSERT(IS_ALIGNED(va_start, PAGE_SIZE_IN_BYTES), "%s: va_start should be aligned\n", __func__);
	ASSERT(IS_ALIGNED(va_stop, PAGE_SIZE_IN_BYTES), "%s: va_stop should be aligned\n", __func__);
	ASSERT(va_start != va_stop, "%s: va_start and va_stop shouldn't be equals\n", __func__);

	if (space->region == NULL)
		return vmm_create_region(space, va_start, va_stop, flags, NULL, NULL);

	struct vmm_region *prev = NULL;
	struct vmm_region *cur = space->region;

	while (cur != NULL) {
		u8 after_prev = prev == NULL || va_start > prev->va_start;
		u8 before_cur = va_stop < cur->va_stop;
		if (after_prev && before_cur)
			break ;
		prev = cur;
		cur = cur->next;
	}

	return vmm_insert_range(space, prev, cur, va_start, va_stop, flags);
}

void vmm_remove_range(struct vmm_space *space, void *va_start, void *va_stop) {
	struct vmm_region *region = space->region;

	while (region != NULL) {
		if (va_start >= region->va_start && va_start < region->va_stop)
			break ;
		region = region->next;
	}

	if (region == NULL)
		panic("%s: not region matching va range %p -> %p", __func__, va_start, va_stop); // TODO: don't panic

	if (va_stop > region->va_stop)
		panic("%s: can't free non contiguous va", __func__);

	// remove the region
	if (region->va_start == va_start && region->va_stop == va_stop)
		return vmm_remove_region(space, region);

	// shrink the region
	if (region->va_start == va_start) {
		region->va_start = va_stop;
		return ;
	}

	if (region->va_stop == va_stop) {
		region->va_stop = va_start;
		return ;
	}

	// split the region
	u64 flags = region->flags;
	void *region_va_start = region->va_start;
	void *region_va_stop = region->va_stop;
	vmm_remove_region(space, region);
	vmm_add_range(space, region_va_start, va_start, flags);
	vmm_add_range(space, va_stop, region_va_stop, flags);
}

// TODO: redesign the va_start and va_stop of a vmm space
struct vmm_space *vmm_create_space(void *va_start, void *va_stop) {
	struct vmm_space *space = block_alloc(TYPE_VMM_SPACE);
	space->region = NULL;
	mmu_init(space);
	return space;
}

void vmm_delete_space(struct vmm_space *space) {
	struct vmm_region *tmp;

	while (space->region != NULL) {
		tmp = space->region;
		space->region = space->region->next;
		block_free(tmp, TYPE_VMM_REGION);
	}

	block_free(space, TYPE_VMM_SPACE);
}

// returns address of first usable byte in va
void *vmm_alloc_between(struct vmm_space *space, void *lower_va, void *upper_va, u64 size_in_pages, u64 flags) {
	// search va range
	u64 size_in_bytes = size_in_pages * PAGE_SIZE_IN_BYTES;
	u64 va = (u64)lower_va;
	u64 padding = 2 * PAGE_SIZE_IN_BYTES;
	struct vmm_region *cur = space->region;
	struct vmm_region *prev = NULL;

	while (cur != NULL) {
		if ((u64)cur->va_stop < (u64)lower_va)
			goto loop_next;

		va = MAX(va, (u64)lower_va);
		if ((u64)cur->va_start - va >= size_in_bytes + padding)
			break ;
		loop_next:
		va = (u64)cur->va_stop;
		prev = cur;
		cur = cur->next;
	}

	if (va + size_in_bytes + padding >= (u64)upper_va)
		return NULL;

	va += PAGE_SIZE_IN_BYTES; // padding

	struct vmm_region *region = vmm_insert_range(space, prev, cur, (void *)va, (void *)(va + size_in_bytes), flags);
	if (region == NULL)
		return NULL;

	for (u64 _va = (u64)va; _va < (u64)region->va_stop; _va += PAGE_SIZE_IN_BYTES) {
		void *pa = pmm_xalloc(); // TODO: replace all pmm_xalloc with pmm_alloc + errors message
		if (mmu_map_page(space, (void *)_va, pa, flags) != 0)
			panic("%s: overwrite of an already used virtual address %p", __func__, _va);
	}

	return (void *)va;
}

// returns address of first usable byte in va
void *vmm_alloc_at(struct vmm_space *space, void *va, u64 size_in_pages, u64 flags) {
	if (va == NULL)
		return NULL;

	struct vmm_region *region = vmm_add_range(space, va, (void *)((u64)va + (size_in_pages * PAGE_SIZE_IN_BYTES)), flags);
	if (region == NULL)
		return NULL;

	for (u64 _va = (u64)va; _va < (u64)region->va_stop; _va += PAGE_SIZE_IN_BYTES) {
		void *pa = pmm_xalloc(); // TODO: replace all pmm_xalloc with pmm_alloc + errors message
		if (mmu_map_page(space, (void *)_va, pa, flags) != 0)
			panic("%s: overwrite of an already used virtual address %p", __func__, _va);
	}
	return va;
}

void *vmm_map(struct vmm_space *space, void *va, void *pa, u64 flags) {
	struct vmm_region *region = vmm_add_range(space, va, (void *)((u64)va + PAGE_SIZE_IN_BYTES), flags);
	if (region == NULL)
		return NULL;
	if (mmu_map_page(space, va, pa, flags) != 0)
		panic("%s: overwrite of an already used virtual address %p", __func__, va);
	return va;
}

void vmm_free(struct vmm_space *space, void *va, u64 size_in_pages) {
	u64 va_stop = (u64)va + (u64)(size_in_pages * PAGE_SIZE_IN_BYTES);

	vmm_remove_range(space, va, (void *)va_stop);

	// make the unmapping effective in the mmu
	for (u64 _va = (u64)va; _va < va_stop; _va += PAGE_SIZE_IN_BYTES) {
		if (mmu_unmap_page(space, (void *)_va) == 0)
			panic("%s: trying to unmap a non mapped va %p", __func__, va);
	}
}

// will map contiguous virtual region with contiguous physical memory
// should only be used to setup kernel code/data regions
static void vmm_init_phys_contiguous_region(struct vmm_region *region, void *pa) {
	for (u64 offset = 0; (u64)region->va_start + offset < (u64)region->va_stop; offset += PAGE_SIZE_IN_BYTES) {
		void *_va = (void *)((u64)region->va_start + offset);
		void *_pa = (void *)((u64)pa + offset);
		if (mmu_map_page(kspace, _va, _pa, region->flags))
			panic("%s: mapping an already mapped entry va=%p | pa=%p", __func__, _va, _pa);
	}
}

// make sure to map all phys available and reserved memory to direct mapping
void vmm_init_direct_mapping(void) {
	struct pmm_phys_addr addr = pmm_next_phys_page(0);

	do {
		u64 flags = addr.type == PMM_REGION_AVAILABLE ? PAGE_KERNEL_RW : PAGE_KERNEL_RO;
		struct vmm_region *region = vmm_add_range(kspace, DM_P2V(addr.pa), DM_P2V(addr.pa) + PAGE_SIZE_IN_BYTES, flags);
		if (region == NULL)
			panic("DAM");
		kernel_dm_stop = DM_P2V(addr.pa);
		if (mmu_map_page(kspace, DM_P2V(addr.pa), addr.pa, flags))
			panic("%s: mapping an already mapped entry va=%p | pa=%p", __func__, DM_P2V(addr.pa), addr.pa);
		addr = pmm_next_phys_page(addr.pa);
	} while (addr.pa != (void *)1); // TODO: cleaner way to return end of pmm
}

void vmm_init(void) {
	// map kernel code/data and direct mapping of available memory
	kspace = vmm_create_space((void *)HIGHER_HALF_START, (void *)HIGHER_HALF_STOP);

	struct vmm_region *kuefi = vmm_add_range(kspace, kernel_vma_rw_start, kernel_vma_rw_stop, PAGE_KERNEL_RW);
	struct vmm_region *kcode = vmm_add_range(kspace, kernel_vma_ro_start, kernel_vma_ro_stop, PAGE_KERNEL_RO);
	struct vmm_region *kdata = vmm_add_range(kspace, kernel_vma_ro_stop, pmm_vma_stop, PAGE_KERNEL_RW);

	vmm_init_phys_contiguous_region(kuefi, (void *)V2P(kuefi->va_start));
	vmm_init_phys_contiguous_region(kcode, (void *)V2P(kcode->va_start));
	vmm_init_phys_contiguous_region(kdata, (void *)V2P(kdata->va_start));

	vmm_init_direct_mapping();

	#ifdef DEBUG
	mmu_test_kernel_mapping(kspace);
	#endif

	mmu_switch_space(kspace);

	#ifdef DEBUG
	vmm_probe_space(kspace);
	#endif
}

// TODO: enhance design
// what to do if something went wrong in the middle of the copy?
// its a general problem in the VMM, if something went wrong in the middle of an action on the 
// page mapping, what should we do?
// there is a lot of possibilites, for the moment we'll stick with instant panics.
void vmm_copy_regions(struct vmm_space *dst, struct vmm_space *src) {
	struct vmm_region *region = src->region;

	while (region != NULL) {
		if (vmm_add_range(dst, region->va_start, region->va_stop, region->flags) == NULL)
			panic("%s: copy on region possibly corrupted", __func__);
		mmu_copy_pages(dst, src, region);
		region = region->next;
	}
}

void vmm_dump_space(struct vmm_space *space) {
	printk("[SPACE]\n");
	printk("---------------------------------------\n");
	for (struct vmm_region *region = space->region; region != NULL; region = region->next) {
		printk("%p -> %p | [%uKiB]\n", region->va_start, region->va_stop, BYTE2KB((u64)region->va_stop - (u64)region->va_start));
	}
}
// END Virtual Memory Manager -----------------------