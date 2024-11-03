#ifndef SYSCALL_H
# define SYSCALL_H

# include "kernel/types.h"
# include "kernel/cpu.h"

void	syscall(const struct trap_frame *tf);

#endif