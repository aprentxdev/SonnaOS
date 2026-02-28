#include <arch/x86_64/mm/vmm.h>
#include <klib/memory.h>
#include <drivers/serial.h>
#include <klib/string.h>

#define PML4_SHIFT 39
#define PDP_SHIFT 30
#define PD_SHIFT 21
#define PT_SHIFT 12

#define PML4_MASK 0x1FF
#define PDP_MASK 0x1FF
#define PD_MASK 0x1FF
#define PT_MASK 0x1FF

#define PML4_INDEX(v) (((v) >> PML4_SHIFT) & PML4_MASK)
#define PDP_INDEX(v) (((v) >> PDP_SHIFT) & PDP_MASK)
#define PD_INDEX(v) (((v) >> PD_SHIFT) & PD_MASK)
#define PT_INDEX(v) (((v) >> PT_SHIFT) & PT_MASK)

#define PAGE_OFFSET(v) ((v) & 0xFFF)

static inline void invlpg(uint64_t addr) {
    asm volatile("invlpg (%0)" ::"r"(addr) : "memory");
}

uint64_t kernel_pml4_phys = 0;

static uint64_t *get_pml4e(uint64_t *pml4, uint64_t virt) {
    return &pml4[PML4_INDEX(virt)];
}

static uint64_t *get_pdpe(uint64_t *pml4, uint64_t virt) {
    uint64_t pml4e = *get_pml4e(pml4, virt);
    if (!(pml4e & PTE_PRESENT)) return NULL;
    uint64_t pdpt_phys = pml4e & ~0xFFFULL;
    uint64_t *pdpt = (uint64_t *)phys_to_virt(pdpt_phys);
    return &pdpt[PDP_INDEX(virt)];
}

static uint64_t *get_pde(uint64_t *pml4, uint64_t virt) {
    uint64_t *pdpe = get_pdpe(pml4, virt);
    if (!pdpe || !(*pdpe & PTE_PRESENT)) return NULL;
    uint64_t pd_phys = *pdpe & ~0xFFFULL;
    uint64_t *pd = (uint64_t *)phys_to_virt(pd_phys);
    return &pd[PD_INDEX(virt)];
}

static uint64_t *get_pte(uint64_t *pml4, uint64_t virt) {
    uint64_t *pde = get_pde(pml4, virt);
    if (!pde || !(*pde & PTE_PRESENT)) return NULL;
    if (*pde & PTE_HUGE) return pde;

    uint64_t pt_phys = *pde & ~0xFFFULL;
    uint64_t *pt = (uint64_t *)phys_to_virt(pt_phys);
    return &pt[PT_INDEX(virt)];
}

static bool create_table(uint64_t *entry, uint64_t flags) {
    if (*entry & PTE_PRESENT) return true;
    void *new_table = pmm_alloc_zeroed();
    if (!new_table) {
        serial_puts("create_table: failed to alloc page table");
        return false;
    }
    uint64_t phys = (uint64_t)new_table;
    *entry = phys | flags | PTE_PRESENT | PTE_WRITE;
    return true;
}

bool vmm_map(uint64_t virt, uint64_t phys, uint64_t flags) {
    if (virt & 0xFFF || phys & 0xFFF) return false;

    uint64_t *pml4 = (uint64_t *)phys_to_virt(kernel_pml4_phys);

    uint64_t *pml4e = get_pml4e(pml4, virt);
    if (!create_table(pml4e, PTE_WRITE)) return false;

    uint64_t *pdpe = get_pdpe(pml4, virt);
    if (!create_table(pdpe, PTE_WRITE)) return false;

    uint64_t *pde = get_pde(pml4, virt);
    if (!create_table(pde, PTE_WRITE)) return false;

    uint64_t *pte = get_pte(pml4, virt);
    if (!pte) return false;

    if (*pte & PTE_PRESENT) {
        serial_puts("vmm_map: already mapped\n");
        return false;
    }

    *pte = phys | (flags & ~(PTE_PRESENT)) | PTE_PRESENT;
    invlpg(virt);
    return true;
}

bool vmm_map_huge_2mb(uint64_t virt, uint64_t phys, uint64_t flags) {
    if (virt & (HUGE_2MB-1) || phys & (HUGE_2MB-1)) return false;

    uint64_t *pml4 = (uint64_t *)phys_to_virt(kernel_pml4_phys);

    uint64_t *pml4e = get_pml4e(pml4, virt);
    if (!create_table(pml4e, PTE_WRITE)) return false;

    uint64_t *pdpe = get_pdpe(pml4, virt);
    if (!create_table(pdpe, PTE_WRITE)) return false;

    uint64_t *pde = get_pde(pml4, virt);
    if (!pde) return false;

    if (*pde & PTE_PRESENT) {
        serial_puts("vmm_map_huge_2mb: already mapped\n");
        return false;
    }

    *pde = phys | (flags & ~(PTE_PRESENT|PTE_HUGE)) | PTE_PRESENT | PTE_HUGE;
    invlpg(virt);
    return true;
}

bool vmm_map_range(uint64_t virt, uint64_t phys, size_t count, uint64_t flags) {
    for (size_t i = 0; i < count; i++) {
        if (!vmm_map(virt + i * PAGE_SIZE, phys + i * PAGE_SIZE, flags)) {
            return false;
        }
    }
    return true;
}

bool vmm_unmap(uint64_t virt) {
    if (virt & 0xFFF) return false;

    uint64_t *pml4 = (uint64_t *)phys_to_virt(kernel_pml4_phys);
    uint64_t *pte = get_pte(pml4, virt);
    if (!pte || !(*pte & PTE_PRESENT)) return false;

    *pte = 0;
    invlpg(virt);
    return true;
}

bool vmm_unmap_huge_2mb(uint64_t virt) {
    if (virt & (HUGE_2MB-1)) return false;

    uint64_t *pml4 = (uint64_t *)phys_to_virt(kernel_pml4_phys);
    uint64_t *pde = get_pde(pml4, virt);
    if (!pde || !(*pde & PTE_PRESENT)) return false;

    *pde = 0;
    invlpg(virt);
    return true;
}

bool vmm_unmap_range(uint64_t virt, size_t count) {
    for (size_t i = 0; i < count; i++) {
        vmm_unmap(virt + i * PAGE_SIZE);
    }
    return true;
}

uint64_t vmm_get_physical(uint64_t virt) {
    uint64_t *pml4 = (uint64_t *)phys_to_virt(kernel_pml4_phys);
    uint64_t *pte = get_pte(pml4, virt);
    if (!pte || !(*pte & PTE_PRESENT)) return 0;

    if (*pte & PTE_HUGE) {
        uint64_t page_phys = *pte & ~(HUGE_2MB-1);
        return page_phys + (virt & (HUGE_2MB-1));
    }

    return (*pte & ~0xFFFULL) + PAGE_OFFSET(virt);
}

uint64_t vmm_get_flags(uint64_t virt) {
    uint64_t *pml4 = (uint64_t *)phys_to_virt(kernel_pml4_phys);
    uint64_t *pte = get_pte(pml4, virt);
    if (!pte) return 0;
    return *pte & 0xFFFULL;
}

void vmm_dump_pte(uint64_t virt) {
    uint64_t phys = vmm_get_physical(virt);
    uint64_t flags = vmm_get_flags(virt);

    char buf[32];
    serial_puts("virt: "); u64_to_hex(virt, buf); serial_puts(buf); serial_puts("\n");
    serial_puts("phys: "); u64_to_hex(phys, buf); serial_puts(buf); serial_puts("\n");
    serial_puts("flags: "); u64_to_hex(flags, buf); serial_puts(buf); serial_puts("\n");
}

void vmm_init(void) {
    uint64_t cr3;
    asm volatile("mov %%cr3, %0" : "=r"(cr3));
    kernel_pml4_phys = cr3 & ~0xFFFULL;
    serial_puts("VMM initialized\n");
}

bool vmm_map_for_pml4(uint64_t *pml4, uint64_t virt, uint64_t phys, uint64_t flags) {
    if (virt & 0xFFF || phys & 0xFFF) return false;

    uint64_t *pml4e = &pml4[PML4_INDEX(virt)];
    if (!(*pml4e & PTE_PRESENT)) {
        void *pdpt = pmm_alloc_zeroed();
        if (!pdpt) return false;
        *pml4e = (uint64_t)pdpt | PTE_PRESENT | PTE_WRITE | PTE_USER;
    }

    uint64_t *pdpt = (uint64_t *)phys_to_virt(*pml4e & ~0xFFFULL);
    uint64_t *pdpe = &pdpt[PDP_INDEX(virt)];
    if (!(*pdpe & PTE_PRESENT)) {
        void *pd = pmm_alloc_zeroed();
        if (!pd) return false;
        *pdpe = (uint64_t)pd | PTE_PRESENT | PTE_WRITE | PTE_USER;
    }

    uint64_t *pd = (uint64_t *)phys_to_virt(*pdpe & ~0xFFFULL);
    uint64_t *pde = &pd[PD_INDEX(virt)];
    if (!(*pde & PTE_PRESENT)) {
        void *pt = pmm_alloc_zeroed();
        if (!pt) return false;
        *pde = (uint64_t)pt | PTE_PRESENT | PTE_WRITE | PTE_USER;
    }

    uint64_t *pt = (uint64_t *)phys_to_virt(*pde & ~0xFFFULL);
    uint64_t *pte = &pt[PT_INDEX(virt)];
    if (*pte & PTE_PRESENT) return false;

    *pte = phys | (flags & ~PTE_PRESENT) | PTE_PRESENT;
    *pte &= ~(1ULL << 63);

    invlpg(virt);
    return true;
}

bool vmm_map_range_for_pml4(uint64_t *pml4, uint64_t virt, uint64_t phys, size_t count, uint64_t flags) {
    for (size_t i = 0; i < count; i++)
        if (!vmm_map_for_pml4(pml4, virt + i*PAGE_SIZE, phys + i*PAGE_SIZE, flags))
            return false;
    return true;
}