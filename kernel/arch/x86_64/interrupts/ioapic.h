#ifndef ESTELLA_ARCH_X86_64_INTERRUPTS_IOAPIC_H
#define ESTELLA_ARCH_X86_64_INTERRUPTS_IOAPIC_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define IOAPIC_ID 0x00
#define IOAPIC_VERSION 0x01
#define IOAPIC_REDIR_TBL_OFFSET 0x10

#define IOREDTBL_MASKED (1ULL << 16)
#define IOREDTBL_TRIGGER_LEVEL (1ULL << 15)
#define IOREDTBL_TRIGGER_EDGE (0ULL << 15)
#define IOREDTBL_POLARITY_LOW (1ULL << 13)
#define IOREDTBL_POLARITY_HIGH (0ULL << 13)

#define IOREDTBL_DELMODE_FIXED (0b000ULL << 8)
#define IOREDTBL_DELMODE_LOWEST (0b001ULL << 8)
#define IOREDTBL_DELMODE_SMI (0b010ULL << 8)
#define IOREDTBL_DELMODE_NMI (0b100ULL << 8)
#define IOREDTBL_DELMODE_INIT (0b101ULL << 8)
#define IOREDTBL_DELMODE_EXTINT (0b111ULL << 8)

typedef struct {
    uint32_t ioapic_id;
    uint32_t gsi_base;
    uint64_t mmio_base_va;
    uint8_t  max_redirection;
} ioapic_t;

extern ioapic_t ioapics[8];
extern size_t ioapic_count;

void ioapic_init_all(void* madt_ptr);
uint32_t ioapic_read(ioapic_t* io, uint32_t reg);
void ioapic_write(ioapic_t* io, uint32_t reg, uint32_t value);
void ioapic_set_irq(uint32_t irq, uint8_t vector, bool level_triggered, bool active_low, uint8_t delivery_mode, uint32_t destination);
void ioapic_mask_irq(uint32_t irq);
void ioapic_unmask_irq(uint32_t irq);

#endif