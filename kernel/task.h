#ifndef PROCESS_H
# define PROCESS_H
# include "kernel/types.h"
# include "kernel/mm/vmm.h"

# include "kernel/sched.h"

# define TASK_NAME_MAX 64

enum process_status {
	TASK_RUNNING,
	TASK_READY,
	TASK_DEAD
};

struct task {
	u16					pid;
	char				name[TASK_NAME_MAX];
	struct vmm_space	*space;
	struct task			*next;
};

struct task	*task_create(void);
void		task_destroy(void);

#endif
