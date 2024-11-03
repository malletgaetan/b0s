// signal.c
#include "kernel/multitasking/signal.h"

i32 signal_send(struct process *proc, u64 signum) {
	if (signum < 0 || signum >= PROCESS_NSIG)
		return -EINVAL;

	// TODO: should we give feedbacks when a signal is masked by a process?
	if (!SIGNAL_IS_SET(proc->masked_signals, signum))
		SIGNAL_SET(proc->pending_signals, signum);

	return 0;
}

void signal_deliver(struct process *proc) {
	if (proc->pending_signals == 0)
		return;

	u64 signum;
	for (signum = 0; signum < PROCESS_NSIG; signum++) {
		if (SIGNAL_IS_SET(proc->pending_signals, signum))
			break;
	}

	SIGNAL_UNSET(proc->pending_signals, signum);

	void (*handler)(u64) = (void (*)(u64))proc->signals_handlers[signum]; // userspace function pointer - to be executed in userspace ONLY

	if (handler == NULL || signum == SIGKILL) {
		switch (signum) {
			case SIGSEGV:
			case SIGKILL:
				// TODO: how do we kill a process hehe
				// TODO: init process (pid 1) shouldn't be killable?
				break;
			case SIGSTOP:
				proc->state = PROCESS_STOPPED;
				break;
			case SIGCONT:
				if (proc->state == PROCESS_STOPPED)
					proc->state = PROCESS_READY;
				break;
		}
		return;
	}

	proc->saved_tf = *proc->tf; // save tf out of user memory space

	proc->tf->rip = (u64)handler;
	proc->tf->rdi = signum; // argument to handler
}
