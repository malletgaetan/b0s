#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

# define TESTER
# undef NULL

#include "kernel/lib/math/math.h"
#include "kernel/lib/bitmap/bitmap.h"
#include "kernel/lib/bitmap/bitmap.c"

int main(void) {
	for (u64 size = 1; size < 10000; size++) {
		u64 bitmap_size_in_bytes = ((size / 64) + 1) * sizeof(u64) + 1;
		u64 *ptr = malloc(bitmap_size_in_bytes);
		u8 *array = malloc(size);
		memset(ptr, 0, bitmap_size_in_bytes);
		memset(array, 0, size);
		struct bitmap bm;
		assert(((void *)bitmap_init(&bm, size, ptr) < (void *)((u64)ptr + bitmap_size_in_bytes)) && "bitmap buffer overflow 1");
		assert(((u8 *)ptr)[bitmap_size_in_bytes - 1] == 0 && "bitmap buffer overflow 2");
		u64 index;
		u64 alloc = 0;
		// take all
		while (1) {
			index = bitmap_find_and_set(&bm);
			if (index == bm.len)
				break ;
			alloc++;
			assert(array[index] == 0 && "bitmap return twice the same place");
			array[index] = 1;
		}

		assert(((u8 *)ptr)[bitmap_size_in_bytes - 1] == 0 && "bitmap buffer overflow 3");
		assert(alloc == size && "bitmap don't handle all allocs");

		// release all
		while (index) {
			bitmap_unset(&bm, --index);
			array[index] = 0;
			alloc--;
			assert(array[index] == 0 && "bitmap return twice the same place");
		}

		assert(((u8 *)ptr)[bitmap_size_in_bytes - 1] == 0 && "bitmap buffer overflow 4");
		assert(alloc == 0 && "bitmap don't handle all frees");

		while (1) {
			index = bitmap_find_and_set(&bm);
			if (index == bm.len)
				break ;
			alloc++;
			assert(array[index] == 0 && "bitmap return twice the same place | realloc");
			array[index] = 1;
		}

		assert(((u8 *)ptr)[bitmap_size_in_bytes - 1] == 0 && "bitmap buffer overflow 5");
		assert(alloc == size && "bitmap don't handle all allocs | realloc");

		free(array);
		free(ptr);
	}
}
