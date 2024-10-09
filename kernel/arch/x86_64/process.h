#ifndef ARCH_PROCESS_H
# define ARCH_PROCESS_H
# include "kernel/types.h"
# include "kernel/process.h"

struct process {
	u16					pid;
	char				name[PROCESS_NAME_MAX];
	enum process_state	state;
	struct vmm_space	*space;
	struct process		*next;
	u8					*kstack;
	struct trap_frame	*tf; // pointer to kstack, used for switch between user and kernel space
	struct context		*context; // pointer to kstack, used for switch between tasks
};


void 	process_switch(struct process *cur, struct process *next);
void 	process_first_run(void);
void 	process_arch_init(struct process *proc, u64 entry);
// void 	process_arch_boot_init(struct process *proc);

#endif