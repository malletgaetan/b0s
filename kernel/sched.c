// #include "kernel/types.h"
// #include "kernel/sched.h"
// #include "kernel/task.h"

// #include "kernel/lib/debug/debug.h"

// static struct task *tasks = NULL;
// static struct task *current_task = NULL;

// void sched_add_task(struct task *task) {
// 	task->next = tasks;
// 	tasks = task;
// }

// u8 sched_remove_task(struct task *task) {
// 	struct task **cur = &tasks;

// 	while (*cur != task) {
// 		if (*cur == NULL)
// 			return 1;
// 		cur = &((*cur)->next);
// 	}
// 	*cur = (*cur)->next;
// 	return 0;
// }

// // returns the task to be ran
// struct task *sched_run(struct cpu_status *context) {
// 	TRACE("%s: start sched", __func__);

// 	current_task->context = context;

// 	struct task *t = current_task->next;
// 	while (t != current_task) {
// 		if (t == NULL) {
// 			t = tasks;
// 			continue ;
// 		}

// 		if (t->status == TASK_READY) {
// 			current_task->status = TASK_READY;
// 			t->status = TASK_RUNNING;
// 			current_task = t;
// 			break ;
// 		}

// 		t = t->next;
// 	}
// 	return current_task;
// }
