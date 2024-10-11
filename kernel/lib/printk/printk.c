#include "kernel/drivers/vga/vga.h"// TODO: a lib shouldn't rely on a specific driver implementation
#include "kernel/types.h"
#include "kernel/lib/printk/printk.h"

# define HEX_ALPHA "0123456789abcdef"

# define LOG_LOOKUP (u8[]){64, 41, 32, 28, 25, 23, 22, 21, 20, 19, 18, 18, 17, 17, 16} // logx(2^64)
# define U64_MAX_LOG_BASE(base) (LOG_LOOKUP[base - 2])


static void putnbr(u64 nbr, u8 base) {
	u8 log = U64_MAX_LOG_BASE(base);
	char array[log];

	u8 i = log - 1;

	do {
		array[i--] = HEX_ALPHA[nbr % base];
		nbr = nbr / base;
	} while (nbr);

	i++;
	while (i <= log - 1)
		vga_putchar(array[i++], VGA_WHITE);
}

static void putptr(u64 ptr) {
	vga_putchar('0', VGA_WHITE);
	vga_putchar('x', VGA_WHITE);

	u8 log = U64_MAX_LOG_BASE(16);
	char array[log];

	u8 i = log - 1;

	do {
		array[i--] = HEX_ALPHA[ptr % 16];
		ptr = ptr / 16;
	} while (i != U8_MAX);

	i++;
	while (i <= log - 1)
		vga_putchar(array[i++], VGA_WHITE);
}

static void putstr(char *str) {
	while (*str)
		vga_putchar(*(str++), VGA_WHITE);
}

static u8 print_arg(char specifier, va_list *ap)
{
	if (specifier == 'b') {
		putnbr(va_arg(*ap, u64), 2);
		return 2;
	}
	if (specifier == 'x') {
		putnbr(va_arg(*ap, u64), 16);
		return 2;
	} else if (specifier == 'u') {
		putnbr(va_arg(*ap, u64), 10);
		return 2;
	} else if (specifier == 'c') {
		vga_putchar((char)va_arg(*ap, int), VGA_WHITE);
		return 2;
	} else if (specifier == 's') {
		putstr(va_arg(*ap, char *));
		return 2;
	} else if (specifier == 'p') {
		putptr(va_arg(*ap, u64));
		return 2;
	} else if (specifier == '%') {
		vga_putchar('%', VGA_WHITE);
		return 2;
	}
	return 1;
}

static void interpret(const char *format, va_list *args)
{
	while (TRUE) {
		while (*format != '%') {
			if ((*format) == '\0')
				return ;
			if ((*format) == '\n') {
				vga_pad();
				// vga_newline();
				format++;
				continue;
			}
			vga_putchar(*format, VGA_WHITE); // TODO: a lib shouldn't rely on a specific driver implementation
			format++;
		}
		format += print_arg(*(format + 1), args);
	}
}

void _printk(const char *format, va_list *args)
{
	if (!format)
		return ;
	interpret(format, args);
}

void printk(const char *format, ...)
{
	va_list args;
	va_start(args, format);
	_printk(format, &args);
	va_end(args);
}