#ifndef BITMAP_H
# define BITMAP_H
# include "kernel/types.h"

struct bitmap {
	u64	len;
	u64	size_in_block;
	u64	*bitmap;
};

u64		*bitmap_init(struct bitmap *bm, u64 len, u64 *ptr);
u64 	*bitmap_init_kheap(struct bitmap *bm, u64 len);
u64		bitmap_find_and_set(struct bitmap *bm);
void	bitmap_unset(struct bitmap *bm, u64 i);

#endif