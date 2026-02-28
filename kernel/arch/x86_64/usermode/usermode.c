#include <arch/x86_64/mm/vmm.h>
#include <arch/x86_64/usermode/usermode.h>
#include <mm/pmm.h>
#include <stdint.h>
#include <klib/memory.h>
#include <drivers/serial.h>
#include <klib/string.h>

extern uint8_t _binary_build_user_code_bin_start[];
extern uint64_t _binary_build_user_code_bin_size;
#define user_code_bin _binary_build_user_code_bin_start
#define user_code_bin_len ((size_t)&_binary_build_user_code_bin_size)

extern uint64_t kernel_pml4_phys;

#define USER_FLAGS_RW  (PTE_PRESENT | PTE_WRITE | PTE_USER)
#define USER_FLAGS_RX  (PTE_PRESENT | PTE_USER)

void enter_usermode(void) {
    // alloc user PML4
    // copy kernel upper-half
    // map code/stack/heap
    // load CR3
    // prepare stack for iretq
    // iretq

    uint64_t *user_pml4_phys = pmm_alloc_zeroed();
    uint64_t *user_pml4 = (uint64_t *)phys_to_virt((uint64_t)user_pml4_phys);

    uint64_t *kernel_pml4 = (uint64_t *)phys_to_virt(kernel_pml4_phys);
    for(int i = 256; i < 512; i++) {
        user_pml4[i] = kernel_pml4[i];
    }

    bool success;

    size_t code_pages = (user_code_bin_len + PAGE_SIZE - 1) / PAGE_SIZE;
    void *user_code_phys = pmm_alloc_frames_zeroed(code_pages);
    success = vmm_map_range_for_pml4(user_pml4, 0x2000000, (uint64_t)user_code_phys, code_pages, USER_FLAGS_RX);
    if (!success) {
        serial_puts("Failed to map code\n");
        return;
    }
    memcpy((void *)phys_to_virt((uint64_t)user_code_phys), user_code_bin, user_code_bin_len);

    void *user_stack_phys = pmm_alloc_zeroed();
    success = vmm_map_for_pml4(user_pml4, 0x3000000, (uint64_t)user_stack_phys, USER_FLAGS_RW);
    if (!success) {
        serial_puts("Failed to map stack\n");
        return;
    }

    void *user_heap_phys = pmm_alloc_zeroed();
    success = vmm_map_for_pml4(user_pml4, 0x4000000, (uint64_t)user_heap_phys, USER_FLAGS_RW);
    if (!success) {
        serial_puts("Failed to map heap\n");
        return;
    }

    uint64_t temp_stack_phys = (uint64_t)pmm_alloc_zeroed();
    uint64_t *sp = (uint64_t *)phys_to_virt(temp_stack_phys);

    sp += PAGE_SIZE / 8;
    *--sp = 0x23;                  // SS
    *--sp = 0x3000000 + PAGE_SIZE; // RSP
    *--sp = 0x202;                 // RFLAGS (IF=1)
    *--sp = 0x1B;                  // CS
    *--sp = 0x2000000;             // RIP

    serial_puts("iretq to usermode\n");
    asm volatile(
        "mov %0, %%cr3\n"
        "mov %1, %%rsp\n"
        "iretq\n"
        :
        : "r"(user_pml4_phys), "r"(sp)
        : "memory"
    );
}