#include "kernel/types.h"
#include "kernel/sched.h"
#include "kernel/task.h"

struct task *tasks = NULL;
struct task *current_task = NULL;

void	sched_add_task(struct task *task) {
	task->next = tasks;
	tasks = task;
}

u8		sched_remove_task(struct task *task) {
	struct task **cur = &tasks;

	while () {
		cur = &((*cur)->next);
	}
}

// noreturn
// what happen if nothing to be ran?
void	sched_run(void) {
	struct task *cur = tasks;

	while (cur != NULL) {
		switch (cur->status) {
			case TASK_DEAD:
				task_destroy(cur);
				break ;
			case TASK_READY:
				current_task = cur;
				// schedule
				break ;
			default:
				break ;
		}
		cur = cur->next;
	}
}
