#ifndef GDT_H
# define GDT_H

// AMD64 Volume 2 section 4.4.8
// https://wiki.osdev.org/Global_Descriptor_Table
// access field
# define GDT_ACCESS_RW 0b00000010
# define GDT_ACCESS_CODE_DATA_SEG 0b00010000
# define GDT_ACCESS_SYSTEM_SEG 0b00000000
# define GDT_ACCESS_CODE 0b00001000
# define GDT_ACCESS_DATA 0b00000000
# define GDT_ACCESS_KERNEL 0b00000000
# define GDT_ACCESS_USER 0b01100000
# define GDT_ACCESS_PRESENT 0b10000000

# define GDT_ACCESS_TSS		0b00001001

// flags field
# define GDT_FLAGS_LONG_MODE 0b0010

# define GDT_NULL_INDEX 0
# define GDT_KERNEL_CODE_INDEX 1
# define GDT_KERNEL_DATA_INDEX 2
# define GDT_USER_CODE_INDEX 3
# define GDT_USER_DATA_INDEX 4
# define GDT_TSS_INDEX 5
# define GDT_SIZE 7 // TSS use 2 element

# ifndef ASM_FILE

# include "kernel/types.h" 

// AMD64 Volume 2 - 4.8.3 System descriptor
struct tss_entry {
    u16		limit;
    u16		base0_15; // address of tss
    u8		base16_23;
    u8		access; // access byte flags
    u8		flags2;
    u8		base24_31;
    u32		base32_63;
    u32		reserved;
} __attribute__((__packed__));

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

struct gdt_descriptor {
	u16 size;
	u64 offset;
} __attribute__ ((packed));

extern struct gdt_entry gdt[GDT_SIZE];
void	gdt_reload(void);
void 	gdt_load_tss(void);
void 	gdt_set_tss_rsp(u64 rsp0);

# endif
#endif