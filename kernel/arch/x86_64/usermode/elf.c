#include <stdint.h>
#include <stdbool.h>

#include <arch/x86_64/mm/vmm.h>
#include <mm/pmm.h>
#include <klib/memory.h>
#include <drivers/serial.h>

#include "elf.h"

bool load_elf(void *elf_data, uint64_t *pml4, uint64_t *entry_point_out) {
    Elf64_Ehdr *ehdr = (Elf64_Ehdr *)elf_data;

    if (memcmp(ehdr->e_ident, "\x7F" "ELF", 4) != 0 ||
        ehdr->e_ident[4] != 2 ||    // 64-bit
        ehdr->e_ident[5] != 1 ||    // Little-endian
        ehdr->e_machine != 0x3E ||  // x86_64
        ehdr->e_type != 2) {        // ET_EXEC
        serial_puts("Invalid ELF header\n");
        return false;
    }

    *entry_point_out = ehdr->e_entry;

    Elf64_Phdr *phdr = (Elf64_Phdr *)(elf_data + ehdr->e_phoff);
    for (uint16_t i = 0; i < ehdr->e_phnum; i++) {
        if (phdr[i].p_type != 1) continue;

        uint64_t vaddr = phdr[i].p_vaddr;
        uint64_t offset = phdr[i].p_offset;
        uint64_t filesz = phdr[i].p_filesz;
        uint64_t memsz = phdr[i].p_memsz;
        [[maybe_unused]] uint64_t align = phdr[i].p_align;

        uint64_t page_start = vaddr & ~(PAGE_SIZE - 1);
        uint64_t page_offset = vaddr - page_start;
        size_t num_pages = (memsz + page_offset + PAGE_SIZE - 1) / PAGE_SIZE;

        void *phys = pmm_alloc_frames_zeroed(num_pages);
        if (!phys) {
            serial_puts("Failed to alloc phys mem for ELF segment\n");
            return false;
        }

        uintptr_t virt_base = phys_to_virt((uintptr_t)phys);
        void *dest = (void *)(virt_base + page_offset);
        memcpy(dest, elf_data + offset, filesz);

        uint64_t pte_flags = PTE_PRESENT | PTE_USER;
        if (phdr[i].p_flags & PF_W) pte_flags |= PTE_WRITE;
        if (!(phdr[i].p_flags & PF_X)) pte_flags |= PTE_NX;

        bool success = vmm_map_range_for_pml4(pml4, page_start, (uint64_t)phys, num_pages, pte_flags);
        if (!success) {
            serial_puts("Failed to map ELF segment\n");
            pmm_free_frames(phys, num_pages);
            return false;
        }
    }

    return true;
}