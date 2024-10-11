NAME = b0s.iso
ELF = b0s.elf
# CFLAGS = -Werror -Wall -Wextra -Wno-unused-local-typedefs -ffreestanding -nostdlib -I ./ -mcmodel=large
CFLAGS = -Wall -Wextra -ffreestanding -nostdlib -I ./ -mcmodel=large
AFLAGS = -f elf64
LDFLAGS =
CC = x86_64-elf-gcc
LD = x86_64-elf-ld
QEMU = qemu-system-x86_64
AS = nasm
EXTENDED_SCRIPT = extended_kernel.ld
QEMU = qemu-system-x86_64
# QEMU_FLAGS = -d int,cpu_reset -D qemu.logs -M smm=off -net none -smp 1 -m 64M -cpu qemu64,+pdpe1gb -M hpet=on --no-reboot
QEMU_FLAGS = -d int,cpu_reset -D qemu.logs -M q35,smm=off -net none -smp 1 -m 128M -cpu host -enable-kvm
ARCH = $(shell $(CC) -dumpmachine | cut -f1 -d-)

C_SRCS = kernel/kmain.c \
		 kernel/drivers/acpi/acpi.c \
		 kernel/drivers/vga/vga.c \
		 kernel/lib/debug/panic.c \
		 kernel/lib/printk/printk.c \
		 kernel/lib/string/strlen.c \
		 kernel/lib/string/memset.c \
		 kernel/lib/string/memcmp.c \
		 kernel/lib/string/memcpy.c \
		 kernel/lib/bitmap/bitmap.c \
		 kernel/mm/pmm.c \
		 kernel/mm/vmm.c \
		 kernel/mm/kheap.c \
		 kernel/multitasking/process.c \
		 kernel/multitasking/sched.c

ASM_SRCS =

ifeq ($(filter $(ARCH),x86_64),)
	$(error Unsupported architecture: $(ARCH))
endif

ASM_SRCS += $(shell find kernel/arch/$(ARCH) -name '*.asm')
C_SRCS += $(shell find kernel/arch/$(ARCH) -name '*.c')

C_OBJS = $(C_SRCS:.c=.o)
ASM_OBJS = $(ASM_SRCS:.asm=.O)
OBJS = $(C_OBJS)
OBJS += $(ASM_OBJS)

ifeq ($(MAKECMDGOALS), debug)
	CFLAGS += -D DEBUG -g
	AFLAGS += -g
	LDFLAGS += -g
endif

all: $(NAME)

$(NAME): $(ELF)
	cp $(ELF) iso/boot/$(ELF)
	grub2-mkrescue -o $@ iso 2> /dev/null

$(ELF): $(OBJS) $(EXTENDED_SCRIPT)
	$(CC) $(LDFLAGS) -T $(EXTENDED_SCRIPT) -o $(ELF) $(OBJS) -nostdlib -lgcc

$(EXTENDED_SCRIPT): kernel.ld
	$(CC) -I ./ -E -x c $< > $(EXTENDED_SCRIPT) || rm -f $(EXTENDED_SCRIPT)

%.O: %.asm
	$(CC) -I ./ -E -x c $< > tmp || rm -f $(tmp)
	$(AS) $(AFLAGS) tmp -o $@
	rm tmp 

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	find . -name *.o -type f -delete
	find . -name *.O -type f -delete
	rm -f $(EXTENDED_SCRIPT)
	rm -f $(ELF)
	rm -f $(NAME)
	rm -f qemu.logs
	find tests -type f ! -name "*.c" -delete

run: $(NAME)
	$(QEMU) $(QEMU_FLAGS) -cdrom $(NAME) 

debug: clean $(NAME)

run-gdb: $(NAME)
	$(QEMU) $(QEMU_FLAGS) -s -S -cdrom $(NAME)

re: clean $(NAME)

tests: $(wildcard tests/*.c)
	@for file in $^; do \
		base=$$(basename $$file .c); \
		echo "Running $$file..."; \
		gcc -I ./ -o tests/$$base $$file; \
		valgrind ./tests/$$base; \
		status=$$?; \
		rm -f tests/$$base; \
		if [ $$status -eq 0 ]; then \
			echo "OK"; \
		else \
			echo "KO"; \
			exit 1; \
		fi; \
	done

test-%:
	@echo "Running tests/$*.c"
	@gcc -I ./ -o tests/$* tests/$*.c
	@valgrind ./tests/$*
	@if [ $$? -eq 0 ]; then \
		echo "OK"; \
	else \
		echo "KO"; \
	fi
	@rm -f ./tests/$*

.PHONY: clean run run-gdb re debug tests
