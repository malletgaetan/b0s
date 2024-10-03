#include "kernel/lib/printk/printk.h"
#include "kernel/lib/debug/debug.h"

void panic(char *fmt, ...) {
	va_list args;
	printk("kernel panic: ");
	va_start(args, fmt);
	_printk(fmt, &args);
	va_end(args);
	printk("\n");
	while(1); // never returns
}