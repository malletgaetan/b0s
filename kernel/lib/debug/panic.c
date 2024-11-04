#include "kernel/cpu.h"

#include "kernel/lib/debug/debug.h"
#include "kernel/lib/printk/printk.h"

void panic(char *fmt, ...)
{
	va_list args;
	printk("kernel panic: ");
	va_start(args, fmt);
	_printk(fmt, &args);
	va_end(args);
	printk("\n");
	while (1)
		cpu_halt();
}