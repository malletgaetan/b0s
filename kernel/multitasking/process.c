#include "kernel/multitasking/process.h"
#include "kernel/mm/kheap.h"
#include "kernel/mm/layout.h"
#include "kernel/mm/paging.h"
#include "kernel/mm/vmm.h"
#include "kernel/multitasking/sched.h"

#include "kernel/lib/bitmap/bitmap.h"
#include "kernel/lib/debug/debug.h"
#include "kernel/lib/string/string.h"

static struct bitmap pid_bitmap;

static u16 process_get_unused_pid(void)
{
	u16 pid = (u16)bitmap_find_and_set(&pid_bitmap);
	if (pid == pid_bitmap.len)
		panic("%s: exhausted all PIDs", __func__); // NOTE: run a ripper ?
	return pid;
}

struct process *process_create(char *name, u64 uentry, struct process *parent)
{
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
	void *ustack = vmm_alloc_at(proc->space, (void *)0xf000, (u64)8, PAGE_USER_RW);
	if (ustack == NULL)
		goto fail_ustack;

	process_arch_init(proc, uentry, 0xffff);
	u64 name_len = strlen(name, PROCESS_NAME_MAX);
	memcpy(proc->name, name, name_len);
	proc->name[name_len] = '\0';
	proc->pid = process_get_unused_pid();
	bitmap_init_static((struct bitmap *)&proc->fdbitmap, PROCESS_MAX_FD);
	proc->sched_list = LIST_HEAD_INIT(proc->sched_list);
	proc->siblings_list = LIST_HEAD_INIT(proc->siblings_list);
	proc->state = PROCESS_READY;
	proc->parent = parent;
	if (parent != NULL && parent->child != NULL) {
		list_add(&proc->siblings_list, &parent->child->siblings_list);
	} else {
		parent->child = proc;
	}
	proc->child = NULL;
	memset(proc->signals_handlers, 0, sizeof(void *) * PROCESS_NSIG);
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
void process_init(void)
{
	ASSERT(PROCESS_NSIG <= ((struct process *)0)->masked_signals,
		   "%s: too much possibilites of signals vs masked_signals bitmap\n", __func__);
	ASSERT(PROCESS_NSIG <= ((struct process *)0)->pending_signals,
		   "%s: too much possibilites of signals vs pending_signals bitmap\n", __func__);
	struct process *proc = kmalloc(sizeof(struct process));
	if (proc == NULL)
		panic("%s: failed to init boot process");
	char name[] = "BOOTSTRAP PROCESS";
	proc->space = kspace;
	proc->pid = 0;
	proc->sched_list = LIST_HEAD_INIT(proc->sched_list);
	memcpy(proc->name, name, sizeof(name));
	proc->tf = NULL; // will never be used
	proc->state = PROCESS_RUNNING;
	sched_init(proc);
	bitmap_init_kheap(&pid_bitmap, U16_MAX);
}