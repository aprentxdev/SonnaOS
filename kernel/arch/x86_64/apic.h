#ifndef ESTELLA_ARCH_X86_64_APIC_H
#define ESTELLA_ARCH_X86_64_APIC_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include <arch/x86_64/acpi.h>

#define LAPIC_TPR 0x080
#define LAPIC_EOI 0x0B0
#define LAPIC_SVR 0x0F0
#define LAPIC_ESR 0x280
#define LAPIC_LVT_TIMER 0x320
#define LAPIC_LVT_THERMAL 0x330
#define LAPIC_LVT_PERF 0x340
#define LAPIC_LVT_LINT0 0x350
#define LAPIC_LVT_LINT1 0x360
#define LAPIC_LVT_ERROR 0x370

#define LAPIC_TIMER_INIT 0x380
#define LAPIC_TIMER_CURR 0x390
#define LAPIC_TIMER_DCR 0x3E0

#define LAPIC_SVR_ENABLE (1 << 8)
#define LAPIC_MASKED (1 << 16)
#define LAPIC_MODE_PERIODIC (1 << 17)

#define LAPIC_TIMER_VECTOR 0x20
#define LAPIC_ERROR_VECTOR 0xFE
#define LAPIC_SPURIOUS_VECTOR 0xFF

extern volatile uint64_t lapic_ticks;
extern uint64_t lapic_va;

#define IOAPIC_BASE_PHYS_DEFAULT    0xFEC00000ULL
#define IOAPIC_ID                   0x00
#define IOAPIC_VERSION              0x01
#define IOAPIC_ARBITRATION          0x02
#define IOAPIC_REDIR_TBL_OFFSET     0x10

#define IOREDTBL_LOW(vector, deliv, dest) \
    ((uint32_t)(vector) | (uint32_t)(deliv) | (uint32_t)(dest))

#define IOREDTBL_HIGH(dest)         ((uint32_t)(dest) << 24)

#define IOREDTBL_MASKED             (1ULL << 16)
#define IOREDTBL_TRIGGER_LEVEL      (1ULL << 15)
#define IOREDTBL_TRIGGER_EDGE       (0ULL << 15)
#define IOREDTBL_POLARITY_LOW       (1ULL << 13)
#define IOREDTBL_POLARITY_HIGH      (0ULL << 13)

#define IOREDTBL_DELMODE_FIXED      (0b000ULL << 8)
#define IOREDTBL_DELMODE_LOWEST     (0b001ULL << 8)
#define IOREDTBL_DELMODE_SMI        (0b010ULL << 8)
#define IOREDTBL_DELMODE_NMI        (0b100ULL << 8)
#define IOREDTBL_DELMODE_INIT       (0b101ULL << 8)
#define IOREDTBL_DELMODE_EXTINT     (0b111ULL << 8)

typedef struct {
    uint64_t redirection_table[24];
    uint32_t ioapic_id;
    uint32_t gsi_base;
    uint64_t mmio_base_va;
    uint8_t  max_redirection;
} ioapic_t;

extern ioapic_t ioapics[8];
extern size_t ioapic_count;


void apic_init();

uint32_t ioapic_read(ioapic_t* io, uint32_t reg);
void ioapic_write(ioapic_t* io, uint32_t reg, uint32_t value);

void ioapic_set_irq(uint32_t irq, uint8_t vector, bool level_triggered, 
                    bool active_low, uint8_t delivery_mode, uint32_t destination);

void ioapic_mask_irq(uint32_t irq);
void ioapic_unmask_irq(uint32_t irq);

#endif