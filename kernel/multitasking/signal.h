#ifndef SIGNAL_H
#define SIGNAL_H

#include "kernel/multitasking/process.h"
#include "kernel/types.h"

// https://dsa.cs.tsinghua.edu.cn/oj/static/unix_signal.html
// https://faculty.cs.niu.edu/~hutchins/csci480/signals.htm

#define SIGNAL_UNSET(signals_bm, signum) ((signals_bm) &= ~(1 << (signum)))
#define SIGNAL_SET(signals_bm, signum) ((signals_bm) |= (1 << (signum)))
#define SIGNAL_IS_SET(signals_bm, signum) ((signals_bm) & (1 << (signum)))

enum {
	SIGKILL, // got killed
	SIGSEGV, // invalid memory reference
	SIGSTOP, // unschedule a process
	SIGCONT, // reschedule a process
};

i32 signal_send(struct process *proc, i32 signum);
void signal_deliver(struct process *proc);

#endif
