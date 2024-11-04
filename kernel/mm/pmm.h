#ifndef PMM_H
#define PMM_H
#include "kernel/lib/bitmap/bitmap.h"
#include "kernel/multiboot2.h"
#include "kernel/types.h"

#define PMAP_SIZE 10 // random high enough number

enum pmm_page_status {
	PMM_PAGE_USED = 0,
	PMM_PAGE_FREE = 1,
};

enum pmm_region_type {
	PMM_REGION_AVAILABLE,
	PMM_REGION_RESERVED,
};

// for the moment we only take into account the available memory,
// don't know if it will become a problem but prefer to keep things as simple as
// possible until I found out why it was a bad idea.
struct pmm_region {
	void *addr;
	u64 size_in_pages;
	enum pmm_region_type type;
	union {
		struct bitmap bitmap;
	} optional;
};

struct pmm_phys_addr {
	enum pmm_region_type type;
	void *pa;
};

u64 pmm_init(const struct multiboot_tag_mmap *mmap);
void *pmm_alloc(void);
void *pmm_xalloc(void);
void pmm_free(void *);
void pmm_dump_pmap(void);
struct pmm_phys_addr pmm_next_phys_page(void *addr);

#endif