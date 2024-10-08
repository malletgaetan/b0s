#ifndef TESTER
#include "kernel/mm/kheap.h"
#include "kernel/lib/bitmap/bitmap.h"
#include "kernel/lib/string/string.h"
#include "kernel/lib/debug/debug.h"
#include "kernel/lib/math/math.h"
#endif

// NOTES: set is 0, unset is 1

// returns first unused address
u64 *bitmap_init(struct bitmap *bm, u64 len, u64 *ptr) {
	bm->len = len;
	bm->size_in_block = (len / 64) + 1;
	bm->bitmap = ptr;
	memset((u8 *)bm->bitmap, 0xff, bm->size_in_block * sizeof(u64));
	return (u64 *)((u64)ptr + bm->size_in_block);
}

u64 *bitmap_init_kheap(struct bitmap *bm, u64 len) {
	bm->len = len;
	bm->size_in_block = (len / 64) + 1;
	bm->bitmap = kmalloc(sizeof(u64) * bm->size_in_block);
	if (bm->bitmap == NULL)
		return NULL;
	memset((u8 *)bm->bitmap, 0xff, bm->size_in_block * sizeof(u64));
	return (u64 *)((u64)bm->bitmap + bm->size_in_block);
}

u64 bitmap_find_and_set(struct bitmap *bm) {
	u64 bit_set_index;
	u64 index;

	for (u64 i = 0; i < bm->size_in_block; i++) {
		if (bm->bitmap[i] == 0)
			continue ;

		bit_set_index = (u64)__builtin_ffsll(bm->bitmap[i]);
		bm->bitmap[i] &= ~((u64)1 << (bit_set_index - 1));
		index = ((i * 64) + (bit_set_index - 1));
		break ;
	}
	return MIN(index, bm->len);
}

void bitmap_unset(struct bitmap *bm, u64 i) {
	if (i >= bm->len)
		return ;
	u64 j = i / 64;
	u64 bit_set_index = i % 64;
	bm->bitmap[j] |= 1 << bit_set_index;
}
