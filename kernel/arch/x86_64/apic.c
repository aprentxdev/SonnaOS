#include <stdint.h>
#include <stdbool.h>

#include <limine.h>

#include <arch/x86_64/acpi.h>
#include <arch/x86_64/apic.h>
#include <arch/x86_64/cpuid.h>
#include <arch/x86_64/msr.h>
#include <mm/vmm.h>
#include <drivers/serial.h>
#include <klib/string.h>
#include <arch/x86_64/io.h>
#include <arch/x86_64/hpet.h>

void ioapic_init_all(void* madt_ptr);

volatile uint64_t lapic_ticks = 0;
uint64_t lapic_phys = 0;
uint64_t lapic_va = 0;

bool x2apic_enabled = false;

uint64_t tsc_frequency_hz = 0;
uint64_t tsc_ticks_per_10ms = 0;

ioapic_t ioapics[8];
size_t ioapic_count = 0;

volatile bool lapic_timer_needed = false;

static inline uint64_t rdtsc(void) {
    uint32_t low, high;
    asm volatile("rdtsc" : "=a"(low), "=d"(high));
    return ((uint64_t)high << 32) | low;
}

uint64_t timer_get_tsc(void) {
    return rdtsc();
}

static inline uint32_t lapic_read(uint32_t reg) {
    if (x2apic_enabled) {
        uint32_t msr = 0x800 + (reg >> 4);
        return (uint32_t)rdmsr(msr);
    } else {
        volatile uint32_t* lapic = (volatile uint32_t*)lapic_va;
        return lapic[reg / 4];
    }
}

static inline void lapic_write(uint32_t reg, uint32_t value) {
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

void lapic_timer_handler(void) {
    lapic_eoi();

    if (!lapic_timer_needed) {
        return;
    }

    lapic_ticks++;
    
    if (x2apic_enabled) {
        uint64_t next_deadline = rdtsc() + tsc_ticks_per_10ms;
        wrmsr(IA32_TSC_DEADLINE, next_deadline);
    }
}

void apic_init() {
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
    bool tsc_deadline_supported = (ecx & (1U << 24)) != 0;

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

    bool tsc_invariant = false;

    cpuid(0x80000000, &eax, &ebx, &ecx, &edx);
    uint32_t max_ext_leaf = eax;

    if (max_ext_leaf >= 0x80000007) {
        cpuid(0x80000007, &eax, &ebx, &ecx, &edx);
        tsc_invariant = (edx & (1 << 8)) != 0;
    }

    if (!tsc_invariant)
        serial_puts("TSC not invariant\n");

    tsc_frequency_hz = 0;
    tsc_ticks_per_10ms = 0;

    cpuid(0, &eax, &ebx, &ecx, &edx);
    uint32_t max_leaf = eax;

    if (max_leaf >= 0x15) {
        cpuid(0x15, &eax, &ebx, &ecx, &edx);

        if (eax && ebx && ecx) {
            tsc_frequency_hz = ((uint64_t)ecx * ebx) / eax;
        }
    }

    if (tsc_frequency_hz == 0 && max_leaf >= 0x16) {
        cpuid(0x16, &eax, &ebx, &ecx, &edx);

        if (eax)
            tsc_frequency_hz = (uint64_t)eax * 1000000ULL;
    }

    if (tsc_frequency_hz) {
        tsc_ticks_per_10ms = tsc_frequency_hz / 100;
        serial_puts("TSC calibrated via CPUID\n");
    } else {
        serial_puts("TSC calibration via CPUID failed, trying via HPET\n");

        hpet_init(rsdp_request.response->address);
        if (hpet_frequency_hz == 0) {
            serial_puts("HPET init failed, no TSC calibration\n");
            return;
        }
        const uint64_t calibration_duration_ns = 100000000ULL;
        const uint64_t ticks_to_wait = (hpet_frequency_hz * calibration_duration_ns) / 1000000000ULL;

        uint64_t hpet_start = hpet_read(HPET_MAIN_COUNTER);
        uint64_t tsc_start = rdtsc();

        while (hpet_read(HPET_MAIN_COUNTER) - hpet_start < ticks_to_wait) {
            asm("pause");
        }

        uint64_t tsc_end = rdtsc();
        uint64_t hpet_end = hpet_read(HPET_MAIN_COUNTER);

        uint64_t hpet_delta = hpet_end - hpet_start;
        uint64_t tsc_delta = tsc_end - tsc_start;

        tsc_frequency_hz = (tsc_delta * hpet_frequency_hz) / hpet_delta;

        tsc_ticks_per_10ms = tsc_frequency_hz / 100ULL;

        serial_puts("TSC calibrated via HPET: ");
        char buf[32];
        u64_to_dec(tsc_frequency_hz, buf);
        serial_puts(buf);
        serial_puts(" Hz\n");
    }


    bool use_tsc_deadline = tsc_deadline_supported && (tsc_frequency_hz != 0) && tsc_invariant;

    lapic_write(LAPIC_ESR, 0);
    lapic_write(LAPIC_TPR, 0);
    lapic_write(LAPIC_SVR, LAPIC_SVR_ENABLE | LAPIC_SPURIOUS_VECTOR);
    lapic_write(LAPIC_LVT_TIMER, LAPIC_MASKED | LAPIC_TIMER_VECTOR);
    lapic_write(LAPIC_LVT_THERMAL, LAPIC_MASKED);
    lapic_write(LAPIC_LVT_PERF, LAPIC_MASKED);
    lapic_write(LAPIC_LVT_LINT0, LAPIC_MASKED);
    lapic_write(LAPIC_LVT_LINT1, LAPIC_MASKED);
    lapic_write(LAPIC_LVT_ERROR, LAPIC_MASKED | LAPIC_ERROR_VECTOR);

    if (use_tsc_deadline) {
        serial_puts("Using TSC-deadline timer\n");

        lapic_write(LAPIC_LVT_TIMER, LAPIC_TIMER_VECTOR | LAPIC_LVT_TIMER_TSC_DEADLINE);

        wrmsr(IA32_TSC_DEADLINE, 0);
        wrmsr(IA32_TSC_DEADLINE, rdtsc() + tsc_ticks_per_10ms);
    } else {
        serial_puts("Using periodic LAPIC timer\n");

        lapic_write(LAPIC_TIMER_DCR, 0b0011);
        lapic_write(LAPIC_LVT_TIMER, LAPIC_TIMER_VECTOR | LAPIC_MODE_PERIODIC);
        lapic_write(LAPIC_TIMER_INIT, 1000000);
    }

    lapic_write(LAPIC_LVT_ERROR, LAPIC_ERROR_VECTOR);

    ioapic_init_all(madt);

    serial_puts("APIC initialized\n");
}

static inline void ioapic_write_internal(volatile uint32_t* base, uint32_t reg, uint32_t value) {
    base[0] = reg;
    base[4] = value;
}

static inline uint32_t ioapic_read_internal(volatile uint32_t* base, uint32_t reg) {
    base[0] = reg;
    return base[4];
}

uint32_t ioapic_read(ioapic_t* io, uint32_t reg) {
    if (!io || !io->mmio_base_va) return 0xFFFFFFFF;
    volatile uint32_t* base = (volatile uint32_t*)io->mmio_base_va;
    return ioapic_read_internal(base, reg);
}

void ioapic_write(ioapic_t* io, uint32_t reg, uint32_t value) {
    if (!io || !io->mmio_base_va) return;
    volatile uint32_t* base = (volatile uint32_t*)io->mmio_base_va;
    ioapic_write_internal(base, reg, value);
}

void ioapic_init_one(struct madt_ioapic* entry) {
    if (ioapic_count >= 8) {
        serial_puts("Too many IOAPICs (>8)\n");
        return;
    }

    uint64_t phys = entry->ioapic_address;
    uint64_t virt = phys + hhdm_offset;

    vmm_map(virt, phys, PTE_PRESENT | PTE_WRITE | PTE_PCD | PTE_PWT);

    ioapic_t* io = &ioapics[ioapic_count++];

    io->ioapic_id = entry->ioapic_id;
    io->gsi_base = entry->global_system_interrupt_base;
    io->mmio_base_va = virt;

    uint32_t version = ioapic_read(io, IOAPIC_VERSION);
    io->max_redirection = (version >> 16) & 0xFF;

    for (int i = 0; i <= io->max_redirection; i++) {
        uint32_t reg_low = IOAPIC_REDIR_TBL_OFFSET + i*2;
        uint32_t reg_high = reg_low + 1;

        ioapic_write(io, reg_low, IOREDTBL_MASKED);
        ioapic_write(io, reg_high, 0);
    }
}

void ioapic_init_all(void* madt_ptr) {
    struct madt* madt = madt_ptr;
    uint8_t* ptr = (uint8_t*)madt + sizeof(struct madt);
    uint8_t* end = (uint8_t*)madt + madt->header.length;

    while (ptr < end) {
        struct madt_entry_header* hdr = (struct madt_entry_header*)ptr;

        switch (hdr->type) {
            case MADT_ENTRY_TYPE_IO_APIC: {
                struct madt_ioapic* io = (struct madt_ioapic*)hdr;
                ioapic_init_one(io);
                break;
            }

            case MADT_ENTRY_TYPE_INTERRUPT_OVERRIDE: {
                // Handle ISO later...
                break;
            }

            // Other types can be ignored for now
        }

        ptr += hdr->length;
    }
}


static ioapic_t* ioapic_for_gsi(uint32_t gsi, uint32_t* pin) {
    for (size_t i = 0; i < ioapic_count; i++) {
        ioapic_t* io = &ioapics[i];
        if (gsi >= io->gsi_base && gsi < io->gsi_base + io->max_redirection + 1) {
            *pin = gsi - io->gsi_base;
            return io;
        }
    }
    return NULL;
}

void ioapic_set_irq(uint32_t irq, uint8_t vector, bool level_triggered,
                    bool active_low, uint8_t delivery_mode, uint32_t destination) {
    uint32_t gsi = irq;
    bool level = level_triggered;
    bool low = active_low;

    uint32_t pin;
    ioapic_t* io = ioapic_for_gsi(gsi, &pin);
    if (!io) {
        return;
    }

    uint32_t low_reg = IOAPIC_REDIR_TBL_OFFSET + pin * 2;
    uint32_t high_reg = low_reg + 1;

    uint32_t low_val = vector | delivery_mode |
                       (level ? IOREDTBL_TRIGGER_LEVEL : IOREDTBL_TRIGGER_EDGE) |
                       (low   ? IOREDTBL_POLARITY_LOW  : IOREDTBL_POLARITY_HIGH);

    uint32_t high_val = destination << 24;

    ioapic_write(io, low_reg,  low_val  | IOREDTBL_MASKED);
    ioapic_write(io, high_reg, high_val);
    ioapic_write(io, low_reg,  low_val);
}

void ioapic_mask_irq(uint32_t irq) {
    uint32_t gsi = irq;
    uint32_t pin;
    ioapic_t* io = ioapic_for_gsi(gsi, &pin);
    if (!io) return;

    uint32_t reg = IOAPIC_REDIR_TBL_OFFSET + pin * 2;
    uint32_t val = ioapic_read(io, reg);
    ioapic_write(io, reg, val | IOREDTBL_MASKED);
}

void ioapic_unmask_irq(uint32_t irq) {
    uint32_t gsi = irq;
    uint32_t pin;
    ioapic_t* io = ioapic_for_gsi(gsi, &pin);
    if (!io) return;

    uint32_t reg = IOAPIC_REDIR_TBL_OFFSET + pin * 2;
    uint32_t val = ioapic_read(io, reg);
    ioapic_write(io, reg, val & ~IOREDTBL_MASKED);
}