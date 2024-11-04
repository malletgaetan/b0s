#ifndef DEBUG_H
#define DEBUG_H
#include "kernel/lib/printk/printk.h"
#include "kernel/types.h"
#include <stdarg.h>

#ifdef DEBUG
#define ASSERT(a, b, ...)              \
	do {                               \
		if (!(a))                      \
			panic((b), ##__VA_ARGS__); \
	} while (0) // https://codecraft.co/2014/11/25/variadic-macros-tricks/
#define TRACE(a, ...)             \
	do {                          \
		printk(a, ##__VA_ARGS__); \
	} while (0)
#else
#define ASSERT(a, b, ...)
#define TRACE(...)
#endif

void panic(char *fmt, ...);

#endif