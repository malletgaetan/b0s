#ifndef SCHED_H
#define SCHED_H

#include "kernel/multitasking/process.h"
#include "kernel/types.h"

void sched_init(struct process *proc);
void sched_add(struct process *proc);
void sched_rmv(struct process *proc);
void sched_switch(void);						// will switch of kernel stack if possible
void sched_set_boot(struct process *boot_proc); // used once for the bootstrap process
extern void sched_start(void);					// will start the periodic timer

#endif