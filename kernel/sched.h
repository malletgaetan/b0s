#ifndef SCHED_H
# define SCHED_H

# include "kernel/types.h"
# include "kernel/process.h"

void	sched_add_process(struct process *process);
void	sched_remove_process(struct process *process);
void	sched_switch(void); // will switch of kernel stack if possible
void 	sched_set_boot(struct process *boot_proc); // used once for the bootstrap process
void 	sched_start(void); // will start the periodic timer

#endif