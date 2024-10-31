#ifndef PROCESS_H
# define PROCESS_H

# include "kernel/types.h"

# define PROCESS_NSIG 32
# define PROCESS_NAME_MAX 64
# define PROCESS_MAX_FD 256
# define PROCESS_KERNEL_STACK_SIZE 16384

# define PROCESS_KERNEL_STACK_TOP(x) ((u64)(x) + PROCESS_KERNEL_STACK_SIZE)


// TODO -> use this in the struct process declared in arch
struct process_general {
	u16								pid;
	char							name[PROCESS_NAME_MAX];
	enum process_state				state;
	struct vmm_space				*space;
	struct list_head				sched_list;
	struct list_head				siblings_list; // all siblings
    struct process					*parent; // process that created this one
	struct process					*child;
	struct fdesc					fds[PROCESS_MAX_FD];
	BITMAP_STATIC(PROCESS_MAX_FD)	fdbitmap;
};

enum process_state {
	PROCESS_RUNNING,
	PROCESS_READY,
	PROCESS_ZOMBIE
};

# if defined(__x86_64__)
# include "kernel/arch/x86_64/multitasking/process.h"
# endif

struct process	*process_create(char *name, struct process *parent, u64 uentry);
void 			process_init(void);

#endif
