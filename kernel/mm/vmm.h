#ifndef VMM_H
# define VMM_H
# include "kernel/types.h"

// TODO: cleanup all prev and next, create a doubled linked list struct
# define MAX_STATIC_REGIONS_PER_SPACE 6

enum {
	VMM_KUEFI_INDEX,
	VMM_KCODE_INDEX,
	VMM_KDATA_INDEX,
	VMM_KDM_INDEX,
};

struct vmm_region {
	void				*va_start; // start of allocated va
	void				*va_stop; // end of allocated va
	struct vmm_region 	*next;
	struct vmm_region 	*prev;
	u64					flags;
};

struct vmm_space {
	void				*page_directory;
	struct vmm_region	*region;
};

extern struct vmm_space *kspace;

void				vmm_init(void);
struct	vmm_space	*vmm_create_space(void *va_start, void *va_stop);
void				vmm_delete_space(struct vmm_space *space);
void 				vmm_copy_regions(struct vmm_space *dst, struct vmm_space *src);
void 				*vmm_map(struct vmm_space *space, void *va, void *pa, u64 flags);
void 				*vmm_alloc_between(struct vmm_space *space, void *lower_va, void *upper_va, u64 size_in_pages, u64 flags);
void 				*vmm_alloc_at(struct vmm_space *space, void *va, u64 size_in_pages, u64 flags);
void 				vmm_free(struct vmm_space *space, void *va, u64 size_in_pages);
void 				vmm_dump_space(struct vmm_space *space);


// implemented in arch/$arch/mmu.c
// require the pt_alloc to be initialized
extern u64	mmu_map_page(struct vmm_space *space, void *va, void *pa, u64 perm);
extern u64	mmu_unmap_page(struct vmm_space *space, void *va);
extern void	mmu_copy_pages(struct vmm_space *dst, struct vmm_space *src, struct vmm_region *region);
extern void	mmu_switch_space(struct vmm_space *space);
extern void mmu_init(struct vmm_space *space);
extern void	mmu_test_kernel_mapping(struct vmm_space *space);
extern void mmu_test_compare_mapping(struct vmm_space *space1, struct vmm_space *space2);

#endif