#include "kernel/task.h"
#include "kernel/mm/kheap.h"
#include "kernel/mm/vmm.h"
#include "kernel/mm/layout.h"

#include "kernel/lib/bitmap/bitmap.h"
#include "kernel/lib/string/string.h"
#include "kernel/lib/debug/debug.h"

static struct bitmap pid_bitmap;

static struct task *current_task = NULL;

void task_yield(void) {
	struct task *t = current_task;
	current_task = t->next;
	task_switch(t, current_task);
}

void debug_task_set_current_task(struct task *t) {
	current_task->next = t;
}

static u16 task_get_unused_pid(void) {
	u16 pid = (u16)bitmap_find_and_set(&pid_bitmap);
	if (pid == 0)
		return task_get_unused_pid();
	if (pid == pid_bitmap.len)
		panic("%s: exhausted all PIDs", __func__); // NOTE: run a ripper ?
	return pid;
}

struct task	*task_create(enum task_type type, char *task_name, u64 entry) {
	struct task *task = kmalloc(sizeof(struct task));
	if (task == NULL)
		return NULL;
	u64 task_name_len = strlen(task_name, TASK_NAME_MAX);
	memcpy(task->name, task_name, task_name_len);
	task->name[task_name_len] = '\0';
	task->pid = task_get_unused_pid();
	task->type = type;
	task->space = kspace; // TODO: alloc for userspace task
	task->kernel_stack = kmalloc(TASK_KERNEL_STACK_SIZE);
	if (task->kernel_stack == NULL) {
		kfree(task);
		return NULL;
	}
	task->kernel_stack_top = (void *)((u64)task->kernel_stack + TASK_KERNEL_STACK_SIZE - 1);
	task_init_context(task, entry);
	task->next = NULL;
	return task;
}

u8 task_destroy(struct task *task) {
	// TODO: for the moment we dont' delete all the pages allocated for the task -> solution is to make a page bitmap allocator for each space
	vmm_delete_space(task->space);
	bitmap_unset(&pid_bitmap, task->pid);
	kfree(task);
	return 0;
}

// there is a task that's been running since the beginning, which is the one currently running the code, make sure we keep track of it
static struct task *task_boot_create(void) {
	struct task *task = kmalloc(sizeof(struct task));
	if (task == NULL)
		panic("%s: failed init first kernel task", __func__);
	memcpy(task->name, "BOOT", 5);
	task->pid = 0;
	task->type = TASK_KERNEL;
	task->space = kspace;
	task_boot_init_context(task);
	task->next = NULL;
	return task;
}

void task_init(void) {
	if (bitmap_init_kheap(&pid_bitmap, U16_MAX) == NULL)
		panic("%s: failed to init PID bitmap", __func__);
	struct task *task_boot = task_boot_create();
	current_task = task_boot;
}