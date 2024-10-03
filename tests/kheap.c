#include <sys/mman.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

# define TESTER
# undef NULL
# define panic(...) assert(NULL)

#include "kernel/types.h"
#include "kernel/mm/paging.h"

struct vmm_space {
};

struct vmm_space *kspace = NULL;

void *vmm_alloc(struct vmm_space *space, void *va, u64 size_in_pages, u64 flags);
void vmm_free(struct vmm_space *space, void *va, u64 size_in_pages);

#include "kernel/mm/kheap.h"
#include "kernel/mm/kheap.c"

u64 total_vmm_allocated = 0;
u64 total_vmm_freed = 0;

void *vmm_alloc(struct vmm_space *space, void *va, u64 size_in_pages, u64 flags) {
	(void)space;
	(void)va;
	(void)flags;
	total_vmm_allocated += size_in_pages;
	return malloc(size_in_pages * PAGE_SIZE_IN_BYTES); // backing with malloc for valgrind debug
}

void vmm_free(struct vmm_space *space, void *va, u64 size_in_pages) {
	(void)space;
	total_vmm_freed += size_in_pages;
	free(va);
}


int random_range(int min, int max) {
	return min + rand() % (max - min + 1);
}

int *random_array(int size) {
	int *array = malloc(size * sizeof(int));
	assert(array != NULL);
	srand(time(NULL));

	// Fill the array with random numbers
	for (int i = 0; i < size; i++) {
		array[i] = random_range(16, 1024);
	}
	return array;
}

int main(void) {
	kheap_init(1);
	for (u64 nb_allocs = 100; nb_allocs < 5000; nb_allocs += 7) {
		int *alloc_array = random_array(nb_allocs);
		void **ptr_array = malloc(nb_allocs * sizeof(void *));
		assert(ptr_array != NULL);
		memset(ptr_array, 0, nb_allocs * sizeof(void *));
		for (u64 i = 0; i < nb_allocs; i++) {
			ptr_array[i] = kmalloc(alloc_array[i]);
			assert(ptr_array[i] != NULL);
			memset(ptr_array[i], 0xff, alloc_array[i]); // try to corrupt
		}
		u64 stop = random_range(2, nb_allocs - 2);
		for (u64 i = 0; i < stop; i++) {
			kfree(ptr_array[i]);
		}
		for (u64 i = 0; i < stop; i++) {
			ptr_array[i] = kmalloc(alloc_array[i]);
			assert(ptr_array[i] != NULL);
			memset(ptr_array[i], 0xff, alloc_array[i]); // try to corrupt
		}
		for (u64 i = 0; i < nb_allocs ; i++) {
			kfree(ptr_array[i]);
		}
		assert(total_vmm_allocated == total_vmm_freed);
		free(alloc_array);
		free(ptr_array);
	}
}