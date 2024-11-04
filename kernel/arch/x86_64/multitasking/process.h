#ifndef ARCH_PROCESS_H
#define ARCH_PROCESS_H
#include "kernel/fs/vfs.h"
#include "kernel/lib/bitmap/bitmap.h"
#include "kernel/multitasking/process.h"
#include "kernel/types.h"

// TODO: find a better way to organize part of process that should only be visible to arch, and part
// that should be visible by all parts of OS
struct process {
	// general part
	u16 pid;
	char name[PROCESS_NAME_MAX];
	enum process_state state;
	struct vmm_space *space;
	struct list_head sched_list;
	struct list_head siblings_list; // all siblings
	struct process *parent;			// process that created this one
	struct process *child;
	struct fdesc fds[PROCESS_MAX_FD];
	BITMAP_STATIC(PROCESS_MAX_FD) fdbitmap;
	u64 masked_signals;
	u64 pending_signals;
	u64 signals_handlers[PROCESS_NSIG];
	// arch part
	u8 *kstack;
	struct trap_frame *tf;		// pointer to kstack, used for switch between user and kernel space
	struct trap_frame saved_tf; // used to save tf out of user stack
	struct context *context;	// pointer to kstack, used for switch between tasks
};

void process_switch(struct process *cur, struct process *next);
void process_first_run(void);
void process_arch_init(struct process *proc, u64 entry, u64 ustack);

#endif