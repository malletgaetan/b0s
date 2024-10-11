#ifndef SCHED_H
# define SCHED_H

# include "kernel/types.h"
# include "kernel/multitasking/process.h"

void 		sched_init(struct process *proc);
void		sched_add_process(struct process *process);
void		sched_remove_process(struct process *process);
void		sched_switch(void); // will switch of kernel stack if possible
void 		sched_set_boot(struct process *boot_proc); // used once for the bootstrap process
extern void	sched_start(void); // will start the periodic timer

#endif