#ifndef ESTELLA_ARCH_X86_64_USERMODE_SCHEDULER_H
#define ESTELLA_ARCH_X86_64_USERMODE_SCHEDULER_H

#include <stdint.h>
#include <stddef.h>

#define TASK_STACK_SIZE (16 * 4096)
#define MAX_TASKS       16

typedef enum {
    TASK_READY,
    TASK_RUNNING,
    TASK_BLOCKED,
    TASK_DEAD,
} task_state_t;

typedef struct {
    uint64_t r15, r14, r13, r12, r11, r10, r9, r8;
    uint64_t rbp, rdi, rsi, rdx, rcx, rbx, rax;
    uint64_t rip, cs, rflags, rsp, ss;
} __attribute__((packed)) cpu_context_t;

typedef struct task {
    cpu_context_t *ctx;
    uint64_t      *pml4_phys;
    void          *kernel_stack;
    task_state_t   state;
    uint32_t       pid;
    struct task   *next;
} task_t;

void scheduler_init(void);
void scheduler_add_task(task_t *task);
void schedule(void);
task_t *scheduler_next(void);
task_t *task_create_from_elf(void *elf_data);
extern task_t *current_task;

#endif