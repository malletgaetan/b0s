#ifndef GDT_H
# define GDT_H

// AMD64 Volume 2 section 4.4.8
// access field
# define GDT_ACCESS_RW 0b00000010
# define GDT_ACCESS_CODE 	0b00011000
# define GDT_ACCESS_DATA 	0b00010000
# define GDT_ACCESS_KERNEL 	0b00000000
# define GDT_ACCESS_USER 	0b01100000
# define GDT_ACCESS_PRESENT 0b10000000

// flags field
# define GDT_FLAGS_LONG_MODE 0b0010

# define GDT_NULL_INDEX 0
# define GDT_KERNEL_CODE_INDEX 1
# define GDT_KERNEL_DATA_INDEX 2
# define GDT_USER_CODE_INDEX 3
# define GDT_USER_DATA_INDEX 4
# define GDT_SIZE 5

# ifndef ASM_FILE

# include "kernel/types.h" 

struct tss {
	u32 reserved0;
	u64 rsp0;
	u64 rsp1;
	u64 rsp2;
	u64 reserved1;
	u64 reserved2;
	u64 ist1;
	u64 ist2;
	u64 ist3;
	u64 ist4;
	u64 ist5;
	u64 ist6;
	u64 ist7;
	u64 reserved3;
	u16	reserved4;
	u16	io_bitmap_offset;
} __attribute__((__packed__));

struct gdt_entry {
	u16	limit0_15; // ignored in long mode
	u16	base0_15; // ignored in long mode
	u8	base16_23; // ignored in long mode
	u8	access;
	u8	limit16_19 : 4; // ignored in long mode
	u8	flags : 4;
	u8	base24_31; // ignored in long mode
} __attribute__ ((packed));


// useless but nice to have
struct gdt_descriptor {
	u16 size;
	u64 offset;
} __attribute__ ((packed));

extern struct gdt_entry gdt[GDT_SIZE];
void	gdt_reload(void);

# endif
#endif