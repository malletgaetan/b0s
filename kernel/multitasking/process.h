#ifndef PROCESS_H
# define PROCESS_H

# include "kernel/types.h"

# define PROCESS_NAME_MAX 64
# define PROCESS_KERNEL_STACK_SIZE 16384

# define PROCESS_KERNEL_STACK_TOP(x) ((u64)(x) + PROCESS_KERNEL_STACK_SIZE)

enum process_state {
	PROCESS_RUNNING,
	PROCESS_READY,
	PROCESS_ZOMBIE
};

# if defined(__x86_64__)
# include "kernel/arch/x86_64/multitasking/process.h"
# endif

struct process	*process_create(char *process_name, u64 uentry);
void 			process_init(void);

#endif
