#include "kernel/arch/x86_64/interrupts/pic.h"
#include "kernel/arch/x86_64/asm.h"

// from https://github.com/dreamportdev/Osdev-Notes/blob/master/02_Architecture/07_APIC.md
void pic_disable(void)
{
	outb(PIC_COMMAND_MASTER, ICW_1);
	outb(PIC_COMMAND_SLAVE, ICW_1);
	outb(PIC_DATA_MASTER, ICW_2_M);
	outb(PIC_DATA_SLAVE, ICW_2_S);
	outb(PIC_DATA_MASTER, ICW_3_M);
	outb(PIC_DATA_SLAVE, ICW_3_S);
	outb(PIC_DATA_MASTER, ICW_4);
	outb(PIC_DATA_SLAVE, ICW_4);
	outb(PIC_DATA_MASTER, 0xFF);
	outb(PIC_DATA_SLAVE, 0xFF);
}