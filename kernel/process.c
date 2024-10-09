#include "kernel/process.h"
#include "kernel/mm/kheap.h"
#include "kernel/mm/vmm.h"
#include "kernel/mm/layout.h"

#include "kernel/lib/bitmap/bitmap.h"
#include "kernel/lib/string/string.h"
#include "kernel/lib/debug/debug.h"

static struct bitmap pid_bitmap;

static u16 process_get_unused_pid(void) {
	u16 pid = (u16)bitmap_find_and_set(&pid_bitmap);
	if (pid == 0)
		return process_get_unused_pid();
	if (pid == pid_bitmap.len)
		panic("%s: exhausted all PIDs", __func__); // NOTE: run a ripper ?
	return pid;
}

struct process *process_create(char *name, u64 entry) {
	struct process *proc = kmalloc(sizeof(struct process));
	if (proc == NULL)
		return NULL;

	proc->kstack = kmalloc(PROCESS_KERNEL_STACK_SIZE);
	if (proc->kstack == NULL)
		goto fail_kstack;
	process_arch_init(proc, entry);

	proc->space = kspace; // TODO: alloc for userspace proc
	proc->space = vmm_create_space((void *)0x1000, (void *)LOWER_HALF_STOP);
	if (proc->space == NULL)
		goto fail_vmm;

	u64 name_len = strlen(name, PROCESS_NAME_MAX);
	memcpy(proc->name, name, name_len);
	proc->name[name_len] = '\0';
	proc->pid = process_get_unused_pid();
	proc->next = NULL;
	return proc;

	fail_vmm:
	kfree(proc->kstack);
	fail_kstack:
	kfree(proc);
	return NULL;
}

// // the boostrapping kernel stack is a process as the others and should be setup
// void process_init(void) {
// 	struct process *proc = kmalloc(sizeof(struct process));
// 	if (proc == NULL)
// 		panic("%s: failed to init boot process");
// 	proc->space = kspace;
// 	proc->pid = 0;
// 	memcpy(proc->name, "BOOTSTRAP/IDLE", 15);
// 	proc->next = NULL;
// 	proc->tf = NULL; // will never be used
// 	process_init_boot_context(proc);
// }