#include <arch/x86_64/usermode/scheduler.h>
#include <mm/pmm.h>
#include <arch/x86_64/mm/vmm.h>
#include <arch/x86_64/cpu/gdt.h>
#include <klib/memory.h>
#include <drivers/serial.h>
#include <arch/x86_64/usermode/elf.h>
#include <klib/string.h>

task_t *current_task = NULL;
static task_t *run_queue_head = NULL;

extern struct tss_struct tss;
extern uint64_t kernel_pml4_phys;

void scheduler_init(void) {
    run_queue_head = NULL;
    current_task   = NULL;
    serial_puts("[scheduler] initialized\n");
}

void scheduler_add_task(task_t *task) {
    task->state = TASK_READY;
    if (!run_queue_head) {
        run_queue_head = task;
        task->next = task;
        return;
    }
    task_t *tail = run_queue_head;
    while (tail->next != run_queue_head) tail = tail->next;
    tail->next = task;
    task->next = run_queue_head;
}

task_t *scheduler_next(void) {
    if (!run_queue_head) return NULL;

    task_t *start = current_task ? current_task->next : run_queue_head;
    task_t *t = start;
    do {
        if (t->state == TASK_READY || t->state == TASK_RUNNING)
            return t;
        t = t->next;
    } while (t != start);

    return NULL;
}

static uint32_t next_pid = 1;

task_t *task_create_from_elf(void *elf_data) {
    task_t *task = (task_t *)phys_to_virt((uint64_t)pmm_alloc_zeroed());
    task->pid = next_pid++;

    uint64_t *pml4_phys = pmm_alloc_zeroed();
    uint64_t *pml4      = (uint64_t *)phys_to_virt((uint64_t)pml4_phys);
    uint64_t *kpml4     = (uint64_t *)phys_to_virt(kernel_pml4_phys);
    for (int i = 256; i < 512; i++) pml4[i] = kpml4[i];
    task->pml4_phys = pml4_phys;

    uint64_t entry = 0;
    if (!load_elf(elf_data, pml4, &entry)) {
        serial_puts("[task] ELF load failed\n");
        return NULL;
    }

    #define USER_STACK_VADDR 0x7FFFFFFF0000ULL
    #define USER_STACK_PAGES 8
    void *ustack_phys = pmm_alloc_frames_zeroed(USER_STACK_PAGES);
    vmm_map_range_for_pml4(pml4, USER_STACK_VADDR, (uint64_t)ustack_phys,
                           USER_STACK_PAGES, PTE_PRESENT | PTE_WRITE | PTE_USER | PTE_NX);

    void *kstack_phys = pmm_alloc_frames_zeroed(TASK_STACK_SIZE / 4096);
    task->kernel_stack = (void *)phys_to_virt((uint64_t)kstack_phys);
    uint64_t kstack_top = (uint64_t)task->kernel_stack + TASK_STACK_SIZE;

    cpu_context_t *ctx = (cpu_context_t *)(kstack_top - sizeof(cpu_context_t));
    memset(ctx, 0, sizeof(cpu_context_t));

    ctx->rip    = entry;
    ctx->cs     = 0x23;  // user code (ring 3)
    ctx->rflags = 0x202; // IF=1
    ctx->rsp    = USER_STACK_VADDR + USER_STACK_PAGES * 4096;
    ctx->ss     = 0x1B;  // user data (ring 3)

    task->ctx = ctx;
    return task;
}

void schedule(void) {
    task_t *next = scheduler_next();
    if (!next || next == current_task) return;

    if (current_task && current_task->state == TASK_RUNNING)
        current_task->state = TASK_READY;

    current_task = next;
    current_task->state = TASK_RUNNING;
}