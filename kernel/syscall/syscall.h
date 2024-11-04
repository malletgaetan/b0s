#ifndef SYSCALL_H
#define SYSCALL_H

#include "kernel/cpu.h"
#include "kernel/types.h"

void syscall(const struct trap_frame *tf);

#endif