#include <arch/x86_64/usermode/usermode.h>
#include <stdint.h>
#include <arch/x86_64/mm/vmm.h>
#include <mm/pmm.h>
#include <klib/memory.h>
#include <klib/string.h>
#include <drivers/serial.h>
#include <arch/x86_64/usermode/elf.h>
#include <arch/x86_64/usermode/scheduler.h>

extern uint64_t kernel_pml4_phys;

#define USER_STACK_VADDR 0x7FFFFFFF0000ULL
#define USER_STACK_SIZE  (8 * PAGE_SIZE)
#define USER_HEAP_VADDR  0x400000000000ULL
#define USER_HEAP_SIZE   PAGE_SIZE

#define USER_FLAGS_RW  (PTE_PRESENT | PTE_WRITE | PTE_USER)
#define USER_FLAGS_RX  (PTE_PRESENT | PTE_USER)

void runElf_ring3(void *elf_data, size_t elf_size) {
    uint64_t *user_pml4_phys = pmm_alloc_zeroed();
    uint64_t *user_pml4 = (uint64_t *)phys_to_virt((uint64_t)user_pml4_phys);

    uint64_t *kernel_pml4 = (uint64_t *)phys_to_virt(kernel_pml4_phys);
    for(int i = 256; i < 512; i++) {
        user_pml4[i] = kernel_pml4[i];
    }

    uint64_t entry_point;
    if(!load_elf(elf_data, user_pml4, &entry_point)) {
        serial_puts("[runElf_ring3] Failed to load ELF\n");
        return;
    }

    size_t stack_pages = USER_STACK_SIZE / PAGE_SIZE;
    void *user_stack_phys = pmm_alloc_frames_zeroed(stack_pages);
    bool success = vmm_map_range_for_pml4(user_pml4, USER_STACK_VADDR, (uint64_t)user_stack_phys, stack_pages, PTE_PRESENT | PTE_WRITE | PTE_USER | PTE_NX);
    if (!success) {
        serial_puts("[runElf_ring3] Failed to map user stack\n");
        return;
    }

    void *user_heap_phys = pmm_alloc_zeroed();
    success = vmm_map_for_pml4(user_pml4, USER_HEAP_VADDR, (uint64_t)user_heap_phys, PTE_PRESENT | PTE_WRITE | PTE_USER | PTE_NX);
    if (!success) {
        serial_puts("[runElf_ring3] Failed to map user heap\n");
        return;
    }

    uint64_t temp_stack_phys = (uint64_t)pmm_alloc_zeroed();
    uint64_t *sp = (uint64_t *)phys_to_virt(temp_stack_phys) + (PAGE_SIZE / sizeof(uint64_t));

    *--sp = 0x1B;                  // SS
    *--sp = USER_STACK_VADDR + USER_STACK_SIZE; // RSP
    *--sp = 0x202;                 // RFLAGS (IF=1)
    *--sp = 0x23;                  // CS
    *--sp = entry_point;           // RIP

    serial_puts("[runElf_ring3] iretq to ring3\n");
    asm volatile(
        "mov %0, %%cr3\n"
        "mov %1, %%rsp\n"
        "iretq\n"
        :
        : "r"(user_pml4_phys), "r"(sp)
        : "memory"
    );
}

void task_enter(task_t *task) {
    current_task = task;
    asm volatile(
        "mov %0, %%cr3\n"
        "mov %1, %%rsp\n"
        "pop %%r15\npop %%r14\npop %%r13\npop %%r12\n"
        "pop %%r11\npop %%r10\npop %%r9\npop %%r8\n"
        "pop %%rbp\npop %%rdi\npop %%rsi\npop %%rdx\n"
        "pop %%rcx\npop %%rbx\npop %%rax\n"
        "iretq\n"
        :
        : "r"(task->pml4_phys), "r"(task->ctx)
        : "memory"
    );
}