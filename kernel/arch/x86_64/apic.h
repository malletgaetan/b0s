#ifndef APIC_H
# define APIC_H

#define IA32_APIC_BASE_MSR 0x1B
#define IA32_APIC_BASE_MSR_BSP 0x100 // Processor is a BSP
#define IA32_APIC_BASE_MSR_ENABLE 0x800

// following x2APIC spec -- Intel Volume 3A Part 1 - 10.12
// support for x2APIC on qemu is quite bad as TCG doesn't support it, kvm needs to be enabled.
# define LOCAL_APIC_ID 0x802 // read-only
# define LOCAL_APIC_VERSION 0x803 // read-only
# define TASK_PRIORITY_REGISTER 0x808 // read/write
# define PROCESSOR_PRIORITY_REGISTER 0x80a // read-only
# define EOI_REGISTER 0x80b // write-only
# define LOGICAL_DESTINATION_REGISTER 0x80d // read-only
# define SPURIOUS_INTERRUPT_VECTOR_REGISTER 0x80f // read/write
# define IN_SERVICE_REGISTER 0x810 // read-only

# define TRIGGER_MODE_REGISTER 0x818 // read-only

# define ERROR_STATUS_REGISTER 0x828 // read/write
# define LVT_CMCI_REGISTER 0x82f // read-write
# define INTERRUPT_COMMAND_REGISTER 0x830 // read-write
# define LVT_TIMER_REGISTER 0x832 // read-write
# define LVT_THERMAL_SENSOR_REGISTER 0x833 // read-write
# define LVT_PERFORMANCE_MONITORING_REGISTER 0x834 // read-write
# define LVT_LINT0_REGISTER 0x835 // read-write
# define LVT_LINT1_REGISTER 0x836 // read-write
# define LVT_ERROR_REGISTER 0x837 // read-write
# define INITIAL_COUNT_REGISTER 0x838 // read/write
# define CURRENT_COUNTER_REGISTER 0x839 // read-only
# define DIVIDE_CONFIGURATION_REGISTER 0x83e // read/write
# define SELF_IPI 0x83f // write-only

# define TIMER_DIVIDE_BY_2 0b0000
# define TIMER_DIVIDE_BY_4 0b0001
# define TIMER_DIVIDE_BY_8 0b0010
# define TIMER_DIVIDE_BY_16 0b0011
# define TIMER_DIVIDE_BY_32 0b1000
# define TIMER_DIVIDE_BY_64 0b1001
# define TIMER_DIVIDE_BY_128 0b1010
# define TIMER_DIVIDE_BY_1 0b1011

# define LVT_MASK (1 << 16)


# define IO_APIC_REGISTER_SELECT 0x0
# define IO_APIC_REGISTER_DATA 0x10

# define IO_APIC_ID 0x00
# define IO_APIC_VER 0x01
# define IO_APIC_ARB 0x02
# define IO_APIC_REDTBL(n) (0x10 + 2 * n)

void	io_apic_init(void);
void	local_apic_init(void);
void	local_apic_timer_calibrate(void);
void 	local_apic_eoi(void);
void 	local_apic_timer_arm(void);

#endif