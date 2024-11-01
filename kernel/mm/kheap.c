#ifndef TESTER
#include "kernel/mm/kheap.h"
#include "kernel/mm/vmm.h"
#include "kernel/mm/paging.h"
#include "kernel/mm/layout.h"

#include "kernel/lib/debug/debug.h"
#endif

#include "kernel/lib/math/math.h"

// TODO: align heap allocations in all allocators: just learned about alignment impact on performance, did not tested it, but it could be nice to impl and compare: https://en.wikipedia.org/wiki/Data_structure_alignment

struct block;

struct container {
	u64 				size_in_bytes;
	u64					used_blocks;
	struct container	*prev; // hmh maybe we do not need this one
	struct container	*next;
	struct block 		*block;
};

struct block {
	u64					size_in_bytes;
	u8					used;
	struct block		*prev;
	struct block		*next;
	struct container	*container;
};

# define BLOCK_DATA(x) ((void *)((u64)x + sizeof(struct block)))
# define DATA_BLOCK(x) ((struct block *)((u64)x - sizeof(struct block)))

# define CONTAINER_DATA(x) ((void *)((u64)x + sizeof(struct container)))
# define DATA_CONTAINER(x) ((struct block *)((u64)x - sizeof(struct container)))

static struct container *heap = NULL;

static struct block *try_alloc_block(struct container *container, u64 size_in_bytes) {
	struct block *b = container->block;

	while (b != NULL) {
		if (b->used == FALSE && b->size_in_bytes >= size_in_bytes)
			goto found_block;
		b = b->next;
	}
	return NULL;
	found_block:
	b->used = TRUE;
	b->container->used_blocks++;

	u64 block_end = (u64)BLOCK_DATA(b) + b->size_in_bytes;
	u64 block_end_after_alloc = (u64)BLOCK_DATA(b) + size_in_bytes;

	u64 next_block_struct_end = (u64)ROUNDUP(block_end_after_alloc + sizeof(struct block), KHEAP_ALIGN_IN_BYTES);
	u64 next_block = next_block_struct_end - sizeof(struct block);

	// not enough size to split the block, just return it
	if (next_block_struct_end >= block_end)
		return b;
	// resize and return the b block, and create a free block after

	struct block *new_b = (struct block *)next_block;
	new_b->size_in_bytes = block_end - next_block_struct_end;
	new_b->prev = b;
	new_b->next = b->next;
	new_b->used = FALSE;
	new_b->container = b->container;

	b->next = new_b;
	if (new_b->next != NULL)
		new_b->next->prev = new_b;
	b->size_in_bytes = size_in_bytes;

	return b;
}

static struct container *create_container(u64 size_in_pages) {
	size_in_pages = MAX(KHEAP_MIN_ALLOC_IN_PAGES, size_in_pages);
	struct container *c = (struct container *)vmm_alloc_between(kspace, (void *)KERNEL_HEAP_START, (void *)KERNEL_HEAP_STOP, size_in_pages, PAGE_KERNEL_RW);
	if (c == NULL)
		return NULL;

	c->size_in_bytes = (size_in_pages * PAGE_SIZE_IN_BYTES) - sizeof(struct container);
	c->used_blocks = 0;
	c->prev = NULL;
	c->next = heap;
	if (c->next != NULL)
		c->next->prev = c;

	heap = c;

	c->block = (struct block *)CONTAINER_DATA(c);
	c->block->size_in_bytes = c->size_in_bytes - sizeof(struct block);
	c->block->used = FALSE;
	c->block->prev = NULL;
	c->block->next = NULL;
	c->block->container = c;

	return c;
}

static void remove_container(struct container *container) {
	if (container->prev == NULL)
		heap = container->next;
	else
		container->prev->next = container->next;

	if (container->next)
		container->next->prev = container->prev;

	vmm_free(kspace, (void *)container, (container->size_in_bytes + sizeof(struct container)) / PAGE_SIZE_IN_BYTES);
}

void kheap_init(u64 size_in_pages) {
	if (create_container(size_in_pages) == NULL)
		panic("%s: failed to init kheap", __func__);
}

void *kmalloc(u64 size_in_bytes) {
	struct container *c = (struct container *)heap;
	struct block *b = NULL;
	while (c != NULL) {
		b = try_alloc_block(c, size_in_bytes);
		if (b != NULL)
			goto block_found;
		c = c->next;
	}
	c = create_container(((size_in_bytes + sizeof(struct block)) / PAGE_SIZE_IN_BYTES) + 2); // aggresively overallocate
	if (c == NULL)
		return NULL;
	b = try_alloc_block(c, size_in_bytes);
	if (b == NULL)
		panic("%s: the impossible is possible", __func__);

	block_found:
	return (void *)BLOCK_DATA(b);
}

void kfree(void *ptr) {
	struct block *b = DATA_BLOCK(ptr);
	u64 total_size = b->size_in_bytes + sizeof(struct block);
	b->used = FALSE;
	b->container->used_blocks--;

	// last block of container
	if (b->container->used_blocks == 0)
		return remove_container(b->container);

	// try to merge with prev one
	if (b->prev != NULL && b->prev->used == FALSE) {
		b->prev->next = b->next;
		b->prev->size_in_bytes += total_size;
		b->prev->next = b->next;
		b = b->prev;
	}

	// try to merge with next one
	if (b->next != NULL && b->next->used == FALSE) {
		b->size_in_bytes += b->next->size_in_bytes + sizeof(struct block);
		b->next = b->next->next;
		if (b->next != NULL)
			b->next->prev = b;
	}
}