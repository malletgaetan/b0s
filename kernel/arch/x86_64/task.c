#include "kernel/mm/vmm.h"


#include "kernel/arch/x86_64/task.h"
#include "kernel/arch/x86_64/cpu.h"
#include "kernel/arch/x86_64/mm/gdt.h"

#include "kernel/lib/string/string.h"
#include "kernel/lib/debug/debug.h"

extern u64 *task_switch_asm(u64 *cur_rsp, u64 *next_rsp); // in task.asm
extern u64 stack_bottom_64;

// its possible task_switch does not returns
void task_switch(struct task *current, struct task *next) {
	TRACE("%s: switching from task %s to %s\n", __func__, current->name, next->name);
	task_switch_asm(&current->rsp, &next->rsp);
}

void task_boot_init_context(struct task *task) {
	task->kernel_stack = (void *)stack_bottom_64;
}

void task_init_context(struct task *task, u64 entry) {
	// task->context = (struct cpu_status *)(task->kernel_stack_top - sizeof(struct cpu_status));
	// memset(task->context, 0, sizeof(struct cpu_status)); // that's not needed

	u64 *stack_top = (u64 *)task->kernel_stack_top + 1;
    *(--stack_top) = entry; // ret is executed at address 0x...120b7
    *(--stack_top) = 0;          // rbx = 0
    *(--stack_top) = 0;          // rbp = 0
    *(--stack_top) = 0;          // r15 = 0
    *(--stack_top) = 0;          // r14 = 0
    *(--stack_top) = 0;          // r13 = 0
    *(--stack_top) = 0;          // r12 = 0
	task->rsp = (u64)stack_top;

    // task->context->rip = entry;
    // task->context->rflags = 0x202; // TODO

	// if (task->type == TASK_KERNEL) {
	// 	task->context->cs = GDT_KERNEL_CODE_INDEX * 8;
	// 	task->context->rsp = task->kernel_stack_top;
	// 	task->context->ss = GDT_KERNEL_DATA_INDEX * 8;
	// } else {
	// 	;
	// 	// TODO
	// 	// task->context->cs = 0x08;
	// 	// task->context->rsp = new_task->kernel_stack_top;
	// 	// task->context->ss = 0x10;
	// }
}