#include "kernel/multitasking/sched.h"
#include "kernel/multitasking/process.h"
#include "kernel/types.h"

#include "kernel/lib/debug/debug.h"
#include "kernel/lib/list/list.h"

static struct list_head scheduled_procs = LIST_HEAD_INIT(scheduled_procs);
static struct process *running = NULL;
struct process *sched_schleeper = NULL;

// process given as parameter should be the process used for the boot part of the OS, and now should
// be used as a schleeper
void sched_init(struct process *proc)
{
	sched_schleeper = proc;
	running = proc;
	sched_add(proc);
}

void sched_add(struct process *proc)
{
	list_add(&proc->sched_list, &scheduled_procs);
}

void sched_rmv(struct process *process)
{
	process->state = PROCESS_ZOMBIE;
}

void sched_switch(void)
{
	struct process *proc;
	struct list_head *pos, *n;

	list_for_each_safe(pos, n, &running->sched_list)
	{
		proc = (struct process *)list_entry(pos, struct process, sched_list);
		if (proc->state == PROCESS_READY) {
			signal_deliver(proc);
			process_switch(running, proc);
			return;
		}
		if (proc->state == PROCESS_ZOMBIE) {
			list_del(&proc->sched_list);
		}
	}
	// if nothing is ready to be ran, run the schleeper
	process_switch(running, sched_schleeper);
}
