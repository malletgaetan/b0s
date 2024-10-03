#ifndef PRINTK_H
# define PRINTK_H
# include <stdarg.h>

void	printk(const char *format, ...);
void 	_printk(const char *format, va_list *args);

#endif