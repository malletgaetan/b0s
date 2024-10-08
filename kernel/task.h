#ifndef PROCESS_H
# define PROCESS_H

# include "kernel/types.h"

# define TASK_NAME_MAX 64
# define TASK_KERNEL_STACK_SIZE 16384

# define TASK_KERNEL_STACK_TOP(x) ((u64)(x) + TASK_KERNEL_STACK_SIZE)

enum task_status {
	TASK_RUNNING,
	TASK_READY,
	TASK_ZOMBIE
};

enum task_type {
	TASK_KERNEL,
	TASK_USER
};

# if defined(__x86_64__)
# include "kernel/arch/x86_64/task.h"
# endif

void			task_init(void);
struct task		*task_create(enum task_type type, char *task_name, u64 entry);
void			task_yield(void);
u8 				task_destroy(struct task *task);


void debug_task_set_current_task(struct task *k);

#endif
