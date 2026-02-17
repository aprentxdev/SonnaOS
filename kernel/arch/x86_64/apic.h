#ifndef ESTELLA_ARCH_X86_64_APIC_H
#define ESTELLA_ARCH_X86_64_APIC_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include <arch/x86_64/acpi.h>

#define LAPIC_TPR           0x080
#define LAPIC_EOI           0x0B0
#define LAPIC_SVR           0x0F0
#define LAPIC_ESR           0x280
#define LAPIC_LVT_TIMER     0x320
#define LAPIC_LVT_THERMAL   0x330
#define LAPIC_LVT_PERF      0x340
#define LAPIC_LVT_LINT0     0x350
#define LAPIC_LVT_LINT1     0x360
#define LAPIC_LVT_ERROR     0x370

#define LAPIC_TIMER_INIT    0x380
#define LAPIC_TIMER_CURR    0x390
#define LAPIC_TIMER_DCR     0x3E0

#define LAPIC_SVR_ENABLE    (1U << 8)
#define LAPIC_MASKED        (1U << 16)
#define LAPIC_MODE_PERIODIC (1U << 17)
#define LAPIC_LVT_TIMER_TSC_DEADLINE (1U << 18)

#define LAPIC_TIMER_VECTOR  0x20
#define LAPIC_ERROR_VECTOR  0xFE
#define LAPIC_SPURIOUS_VECTOR 0xFF

#define IA32_TSC_DEADLINE   0x6E0

#define IOAPIC_ID                   0x00
#define IOAPIC_VERSION              0x01
#define IOAPIC_REDIR_TBL_OFFSET     0x10

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
    uint32_t ioapic_id;
    uint32_t gsi_base;
    uint64_t mmio_base_va;
    uint8_t  max_redirection;
} ioapic_t;

extern ioapic_t ioapics[8];
extern size_t ioapic_count;
extern volatile uint64_t lapic_ticks;

extern bool x2apic_enabled;

extern uint64_t tsc_frequency_hz;
extern uint64_t tsc_ticks_per_10ms;

void lapic_eoi(void);

void apic_init(void);

uint32_t ioapic_read(ioapic_t* io, uint32_t reg);
void ioapic_write(ioapic_t* io, uint32_t reg, uint32_t value);

void ioapic_set_irq(uint32_t irq, uint8_t vector, bool level_triggered,
                    bool active_low, uint8_t delivery_mode, uint32_t x2apic_dest);

void ioapic_mask_irq(uint32_t irq);
void ioapic_unmask_irq(uint32_t irq);

extern volatile bool lapic_timer_needed;
uint64_t timer_get_tsc(void);

#endif