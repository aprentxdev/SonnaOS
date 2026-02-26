#include <arch/x86_64/lapic.h>
#include <arch/x86_64/acpi.h>
#include <arch/x86_64/msr.h>
#include <arch/x86_64/cpuid.h>
#include <mm/vmm.h>
#include <drivers/serial.h>


#include <limine.h>

extern volatile struct limine_rsdp_request rsdp_request;
extern uint64_t hhdm_offset;

volatile uint64_t lapic_ticks = 0;
uint64_t lapic_phys = 0;
uint64_t lapic_va = 0;
bool x2apic_enabled = false;

uint32_t lapic_read(uint32_t reg) {
    if (x2apic_enabled) {
        uint32_t msr = 0x800 + (reg >> 4);
        return (uint32_t)rdmsr(msr);
    } else {
        volatile uint32_t* lapic = (volatile uint32_t*)lapic_va;
        return lapic[reg / 4];
    }
}

void lapic_write(uint32_t reg, uint32_t value) {
    if (x2apic_enabled) {
        uint32_t msr = 0x800 + (reg >> 4);
        wrmsr(msr, value);
    } else {
        volatile uint32_t* lapic = (volatile uint32_t*)lapic_va;
        lapic[reg / 4] = value;
    }
}

void lapic_eoi(void) {
    lapic_write(LAPIC_EOI, 0);
}

void lapic_init(void) {
    void *rsdp_ptr = rsdp_request.response->address;
    struct madt* madt = acpi_get_madt(rsdp_ptr);
    if (!madt) {
        return;
    }

    lapic_phys = madt->lapic_address;
    lapic_va = lapic_phys + hhdm_offset;

    vmm_map(lapic_va, lapic_phys, PTE_KERNEL_RW | PTE_PCD | PTE_PWT);

    uint32_t eax, ebx, ecx, edx;
    cpuid(1, &eax, &ebx, &ecx, &edx);

    bool x2apic_supported = (ecx & (1U << 21)) != 0;

    if (x2apic_supported) {
        serial_puts("x2APIC supported\n");

        uint64_t apic_base = rdmsr(IA32_APIC_BASE_MSR);
        apic_base |= IA32_APIC_BASE_ENABLE | IA32_APIC_BASE_X2APIC;
        wrmsr(IA32_APIC_BASE_MSR, apic_base);

        x2apic_enabled = true;
    } else {
        serial_puts("x2APIC not supported\n");
        x2apic_enabled = false;
        return;
    }

    lapic_write(LAPIC_ESR, 0);
    lapic_write(LAPIC_TPR, 0);
    lapic_write(LAPIC_SVR, LAPIC_SVR_ENABLE | LAPIC_SPURIOUS_VECTOR);
    lapic_write(LAPIC_LVT_THERMAL, LAPIC_MASKED);
    lapic_write(LAPIC_LVT_PERF, LAPIC_MASKED);
    lapic_write(LAPIC_LVT_LINT0, LAPIC_MASKED);
    lapic_write(LAPIC_LVT_LINT1, LAPIC_MASKED);
    lapic_write(LAPIC_LVT_ERROR, LAPIC_MASKED | LAPIC_ERROR_VECTOR);
    
    serial_puts("LAPIC initialized\n");
}