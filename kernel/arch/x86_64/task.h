#ifndef ARCH_TASK_H
# define ARCH_TASK_H
# include "kernel/types.h"
# include "kernel/task.h"

struct task {
	u16					pid;
	char				name[TASK_NAME_MAX];
	u64					rsp;
	enum task_status	status;
	enum task_type		type;
	struct cpu_status	*context; // pointer to the kernel stack
	struct vmm_space	*space;
	struct task			*next;
	void				*kernel_stack;
	void				*kernel_stack_top;
};

void			task_init_context(struct task *task, u64 entry);
void 			task_boot_init_context(struct task *task);
void			task_switch(struct task *current, struct task *next);

#endif