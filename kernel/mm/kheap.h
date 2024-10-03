#ifndef KHEAP_H
# define KHEAP_H
# include "kernel/types.h"

# define KHEAP_MIN_ALLOC_IN_PAGES 5

void	kheap_init(u64 size_in_pages);
void	*kmalloc(u64 size);
void	krealloc(void *ptr, u64 new_size);
void	kfree(void *ptr);

#endif