#include <arch/x86_64/hpet.h>
#include <arch/x86_64/acpi.h>
#include <mm/vmm.h>
#include <drivers/serial.h>
#include <klib/string.h>

uint64_t hpet_va = 0;
uint64_t hpet_frequency_hz = 0;

extern uint64_t hhdm_offset;

inline uint64_t hpet_read(uint64_t offset) {
    return *(volatile uint64_t *)(hpet_va + offset);
}

inline void hpet_write(uint64_t offset, uint64_t value) {
    *(volatile uint64_t *)(hpet_va + offset) = value;
}

void hpet_init(void *rsdp_ptr) {
    struct hpet *hpet = acpi_get_hpet(rsdp_ptr);
    if (!hpet) {
        serial_puts("HPET not found\n");
        return;
    }

    uint64_t hpet_phys = hpet->base_address.address;
    hpet_va = hpet_phys + hhdm_offset;

    vmm_map(hpet_va, hpet_phys, PTE_KERNEL_RW | PTE_PCD | PTE_PWT);

    uint64_t capabilities = hpet_read(HPET_CAPABILITIES);
    uint32_t period_fs = (capabilities >> 32) & 0xFFFFFFFF;
    if (period_fs == 0) {
        serial_puts("Invalid HPET period\n");
        return;
    }
    hpet_frequency_hz = 1000000000000000ULL / period_fs;

    hpet_write(HPET_CONFIG, hpet_read(HPET_CONFIG) | HPET_CFG_ENABLE);

    serial_puts("HPET initialized\n");
}