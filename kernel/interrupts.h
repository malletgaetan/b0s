#ifndef INTERRUPTS_H
# define INTERRUPTS_H

// defined in kernel/arch/$arch/interrupts.c
extern void interrupts_entry_init(void);
extern void interrupts_init(void);

#endif