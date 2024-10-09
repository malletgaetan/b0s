#include "kernel/types.h"
#include "kernel/sched.h"
#include "kernel/process.h"

#include "kernel/lib/debug/debug.h"

static struct process *scheduled_procs = NULL;

static struct process *running = NULL;

void sched_set_boot(struct process *boot_proc) {
	running = boot_proc;
	scheduled_procs = boot_proc;
}

void sched_add_process(struct process *proc) {
	scheduled_procs->next = proc;
	scheduled_procs = proc;
}

void sched_remove_process(struct process *proc) {
	struct process **cur = &scheduled_procs; // linus be proud

	while (*cur != proc) {
		if (*cur == NULL)
			return ;
		cur = &((*cur)->next);
	}
	*cur = (*cur)->next;
}

void sched_switch(void) {
	struct process *proc = scheduled_procs;

	while (proc != NULL) {
		if (proc->state == PROCESS_READY) {
			process_switch(running, proc);
			return ;
		}
		proc = proc->next;
	}
}

void sched_start(void); // will start the periodic timer