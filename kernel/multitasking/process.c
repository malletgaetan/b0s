#include "kernel/multitasking/process.h"
#include "kernel/multitasking/sched.h"
#include "kernel/mm/kheap.h"
#include "kernel/mm/vmm.h"
#include "kernel/mm/layout.h"
#include "kernel/mm/paging.h"

#include "kernel/lib/bitmap/bitmap.h"
#include "kernel/lib/string/string.h"
#include "kernel/lib/debug/debug.h"

static struct bitmap pid_bitmap;

static u16 process_get_unused_pid(void) {
	u16 pid = (u16)bitmap_find_and_set(&pid_bitmap);
	if (pid == 0) // NOTE: for the moment bitmap doesn't return 0 first, this will change
		return process_get_unused_pid();
	if (pid == pid_bitmap.len)
		panic("%s: exhausted all PIDs", __func__); // NOTE: run a ripper ?
	return pid;
}

struct process *process_create(char *name, struct process *parent, u64 uentry) {
	struct process *proc = kmalloc(sizeof(struct process));
	if (proc == NULL)
		return NULL;

	proc->kstack = kmalloc(PROCESS_KERNEL_STACK_SIZE);
	if (proc->kstack == NULL)
		goto fail_kstack;

	proc->space = kspace; // TODO: alloc for userspace proc
	proc->space = vmm_create_space((void *)0x1000, (void *)LOWER_HALF_STOP);
	if (proc->space == NULL)
		goto fail_vmm;

	vmm_copy_regions(proc->space, kspace);
	#ifdef DEBUG
	mmu_test_compare_mapping(kspace, proc->space);
	#endif
	u64 ustack = (u64)vmm_alloc_at(proc->space, (void *)0xf000, (u64)8, PAGE_USER_RW);
	if (ustack == NULL)
		goto fail_ustack;

	process_arch_init(proc, uentry, 0xffff);
	u64 name_len = strlen(name, PROCESS_NAME_MAX);
	memcpy(proc->name, name, name_len);
	proc->name[name_len] = '\0';
	proc->pid = process_get_unused_pid();
	bitmap_init(&proc->fdbitmap, PROCESS_MAX_FD);
	proc->sched_list = LIST_HEAD_INIT(proc->sched_list);
	proc->childrens_list = LIST_HEAD_INIT(proc->childrens_list);
	proc->siblings_list = LIST_HEAD_INIT(proc->siblings_list);
	proc->state = PROCESS_READY;
	proc->parent = parent;
	if (parent != NULL && parent->child != NULL) {
		list_add(&proc->siblings_list, parent->child);
	} else {
		parent->child = proc;
	}
	proc->child = NULL;
	return proc;

	fail_ustack:
	vmm_delete_space(proc->space);
	fail_vmm:
	kfree(proc->kstack);
	fail_kstack:
	kfree(proc);
	return NULL;
}

// the boostrapping kernel stack is a process as the others and should be setup
void process_init(void) {
	struct process *proc = kmalloc(sizeof(struct process));
	if (proc == NULL)
		panic("%s: failed to init boot process");
	char name[] = "BOOTSTRAP PROCESS";
	proc->space = kspace;
	proc->pid = 0;
	memcpy(proc->name, name, sizeof(name));
	proc->next = NULL;
	proc->tf = NULL; // will never be used
	proc->state = PROCESS_RUNNING;
	sched_init(proc);
	bitmap_init_kheap(&pid_bitmap, U16_MAX);
}