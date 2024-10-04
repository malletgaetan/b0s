#ifndef SCHED_H
# define SCHED_H
# include "kernel/types.h"

# include "kernel/task.h"

void	sched_add_task(struct task *task);
u8		sched_remove_task(struct task *task);
void	sched_run(void);

#endif