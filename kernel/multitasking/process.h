#ifndef PROCESS_H
#define PROCESS_H

#include "kernel/types.h"

#define PROCESS_NSIG 64
#define PROCESS_NAME_MAX 64
#define PROCESS_MAX_FD 256
#define PROCESS_KERNEL_STACK_SIZE 16384

#define PROCESS_KERNEL_STACK_TOP(x) ((u64)(x) + PROCESS_KERNEL_STACK_SIZE)

// TODO -> use non-arch oriented struct to store general purpose fields of process here

enum process_state {
	PROCESS_RUNNING,
	PROCESS_READY,
	PROCESS_STOPPED,
	PROCESS_ZOMBIE,
};

#if defined(__x86_64__)
#include "kernel/arch/x86_64/multitasking/process.h"
#endif

struct process *process_create(char *name, u64 uentry, struct process *parent);
void process_init(void);

#endif
