#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <time.h>

#define TESTER
#define panic(...) assert(NULL)
#define KERNEL_HEAP_START ((u64)0)
#define KERNEL_HEAP_STOP ((u64)0)

#include "kernel/lib/math/math.h"
#include "kernel/mm/paging.h"
#include "kernel/types.h"

struct vmm_space;

struct vmm_space *kspace = NULL;

void *vmm_alloc_between(struct vmm_space *space, void *lower_va, void *upper_va, u64 size_in_pages,
						u64 flags);
void vmm_free(struct vmm_space *space, void *va, u64 size_in_pages);

#include "kernel/mm/kheap.c"
#include "kernel/mm/kheap.h"

u64 total_vmm_allocated = 0;
u64 total_vmm_freed = 0;

void *vmm_alloc_between(struct vmm_space *space, void *lower_va, void *upper_va, u64 size_in_pages,
						u64 flags)
{
	(void)space;
	(void)lower_va;
	(void)upper_va;
	(void)flags;
	total_vmm_allocated += size_in_pages;
	return malloc(size_in_pages * PAGE_SIZE_IN_BYTES); // backing with malloc for valgrind debug
}

void vmm_free(struct vmm_space *space, void *va, u64 size_in_pages)
{
	(void)space;
	total_vmm_freed += size_in_pages;
	free(va);
}

int random_range(int min, int max)
{
	return min + rand() % (max - min + 1);
}

int *random_array(int size)
{
	int *array = malloc(size * sizeof(int));
	assert(array != NULL);
	srand(time(NULL));

	// Fill the array with random numbers
	for (int i = 0; i < size; i++) {
		array[i] = random_range(16, 1024);
	}
	return array;
}

void stress_test(void)
{
	for (u64 nb_allocs = 100; nb_allocs < 5000; nb_allocs += 7) {
		int *alloc_array = random_array(nb_allocs);
		void **ptr_array = malloc(nb_allocs * sizeof(void *));
		assert(ptr_array != NULL);
		memset(ptr_array, 0, nb_allocs * sizeof(void *));
		for (u64 i = 0; i < nb_allocs; i++) {
			ptr_array[i] = kmalloc(alloc_array[i]);
			assert(ptr_array[i] != NULL);
			assert(IS_ALIGNED(ptr_array[i], KHEAP_ALIGN_IN_BYTES));
			memset(ptr_array[i], 0xff, alloc_array[i]); // try to corrupt
		}
		u64 stop = random_range(2, nb_allocs - 2);
		for (u64 i = 0; i < stop; i++) {
			kfree(ptr_array[i]);
		}
		for (u64 i = 0; i < stop; i++) {
			ptr_array[i] = kmalloc(alloc_array[i]);
			assert(ptr_array[i] != NULL);
			assert(IS_ALIGNED(ptr_array[i], KHEAP_ALIGN_IN_BYTES));
			memset(ptr_array[i], 0xff, alloc_array[i]); // try to corrupt
		}
		for (u64 i = 0; i < nb_allocs; i++) {
			kfree(ptr_array[i]);
		}
		assert(total_vmm_allocated == total_vmm_freed);
		free(alloc_array);
		free(ptr_array);
	}
}

void big_alloc_test(void)
{
	void *ptr = kmalloc(65536); // really big stack
	assert(ptr != NULL);
	memset(ptr, 0xff, 65536); // try to corrupt
	kfree(ptr);
}

int main(void)
{
	kheap_init(1);
	stress_test();
	big_alloc_test();
}