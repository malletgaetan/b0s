#include "kernel/types.h"
#include "kernel/multitasking/sched.h"
#include "kernel/multitasking/process.h"

#include "kernel/lib/debug/debug.h"

static struct process *scheduled_procs = NULL;

static struct list_head scheduled_procs = LIST_HEAD_INIT(scheduled_procs);

static struct process *running = NULL;

void sched_init(struct process *proc) {
	list_add(&proc->sched_list, &scheduled_procs);
	running = proc;
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
