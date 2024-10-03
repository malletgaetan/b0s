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
	void				*va_start;
	void				*va_stop;
	struct vmm_region 	*next;
	struct vmm_region 	*prev;
	u64					flags;
};

struct vmm_space {
	void				*va_start;
	void				*va_stop;
	void				*page_directory;
	struct vmm_region	*region;
};

extern struct vmm_space *kspace;

void	vmm_init(void);
void 	*vmm_map(struct vmm_space *space, void *va, void *pa, u64 flags);
void 	*vmm_alloc(struct vmm_space *space, void *va, u64 size_in_pages, u64 flags);
void 	vmm_free(struct vmm_space *space, void *va, u64 size_in_pages);
void 	vmm_dump_space(struct vmm_space *space);

// implemented in arch/$arch/mmu.c
// require the pt_alloc to be initialized
u64		mmu_map_page(struct vmm_space *space, void *va, void *pa, u64 flags);
u64		mmu_unmap_page(struct vmm_space *space, void *va);
void	mmu_switch_space(struct vmm_space *space);
void 	mmu_init(struct vmm_space *space);
void 	mmu_test_kernel_mapping(struct vmm_space *space);

#endif