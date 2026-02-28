#ifndef ESTELLA_GENERIC_IRQ_H
#define ESTELLA_GENERIC_IRQ_H

#include <stdint.h>
#include <stdbool.h>

#define IRQ_MASKED (1ULL << 16)

#define IRQ_TRIGGER_LEVEL (1ULL << 15)
#define IRQ_TRIGGER_EDGE  (0ULL << 15)

#define IRQ_POLARITY_LOW  (1ULL << 13)
#define IRQ_POLARITY_HIGH (0ULL << 13)

#define IRQ_DELMODE_FIXED   (0b000ULL << 8)
#define IRQ_DELMODE_LOWEST  (0b001ULL << 8)
#define IRQ_DELMODE_SMI     (0b010ULL << 8)
#define IRQ_DELMODE_NMI     (0b100ULL << 8)
#define IRQ_DELMODE_INIT    (0b101ULL << 8)
#define IRQ_DELMODE_EXTINT  (0b111ULL << 8)

void irq_eoi(void);
void irq_set(uint32_t irq, uint8_t vector, bool level_triggered, bool active_low, uint8_t delivery_mode, uint32_t destination);
void irq_mask(uint32_t irq);
void irq_unmask(uint32_t irq);

#endif