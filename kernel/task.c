#include "kernel/task.h"
#include "kernel/mm/kheap.h"
#include "kernel/mm/vmm.h"
#include "kernel/mm/layout.h"

#include "kernel/lib/bitmap/bitmap.h"
#include "kernel/lib/debug/debug.h"

struct bitmap pid_bitmap;

static u16 task_get_unused_pid(void) {
	u16 pid = (u16)bitmap_find_and_set(&pid_bitmap);
	if (pid == pid_bitmap.len)
		panic("%s: exhausted all PIDs", __func__);
	return pid;
}

struct task	*task_create(void) {
	struct task *task = kmalloc(sizeof(struct task));
	if (task == NULL)
		return NULL;
	task->pid = task_get_unused_pid();
	task->space = vmm_create_space(LOWER_HALF_START, LOWER_HALF_STOP);
	if (space == NULL) {
		kfree(task);
		return NULL;
	}
	task->next = NULL;
	return task;
}

u8 task_destroy(struct task *task) {
	if (sched_unschedule(task))
		return 1;
	vmm_delete_space(task->space);
	bitmap_unset(&pid_bitmap, task->pid);
	kfree(task);
}

void task_init(void) {
	if (bitmap_init_kheap(&pid_bitmap, U16_MAX) == NULL)
		panic("%s: failed to init PID bitmap", __func__);
}