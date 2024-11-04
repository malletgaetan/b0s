#include "kernel/drivers/vga/vga.h"

static u8 x = 0;
static u8 y = 0;

u8 vga_color_entry(u8 fg, u8 bg)
{
	return fg | (bg << 4);
}

u16 vga_entry(char ch, u8 color)
{
	return (u16)ch | ((u16)color << 8);
}

void vga_reset(void)
{
	for (u16 i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++)
		vga_putchar(' ', vga_color_entry(VGA_BLACK, VGA_BLACK));
	x = 0;
	y = 0;
}

void vga_pad(void)
{
	u8 _y = y;
	while (y == _y)
		vga_putchar(' ', vga_color_entry(VGA_BLACK, VGA_BLACK));
}

void vga_newline(void)
{
	y = (y + 1) % VGA_HEIGHT;
	x = 0;
}

void vga_putchar(char ch, u8 color)
{
	u16 *vga_buffer = (u16 *)VGA_BUFFER_ADDRESS;
	u32 index = (y * VGA_WIDTH) + x;
	vga_buffer[index] = vga_entry(ch, color);
	y = (y + ((x + 1) / VGA_WIDTH)) % VGA_HEIGHT;
	x = (x + 1) % VGA_WIDTH;
}
