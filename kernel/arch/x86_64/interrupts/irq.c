#include <generic/irq.h>
#include <arch/x86_64/interrupts/apic.h>
#include <arch/x86_64/interrupts/ioapic.h>
#include <arch/x86_64/interrupts/lapic.h>

void irq_set(uint32_t irq, uint8_t vector, bool level, bool low, uint8_t mode, uint32_t dest) {
    ioapic_set_irq(irq, vector, level, low, mode, dest);
}

void irq_eoi(void) {
    lapic_eoi();
}

void irq_mask(uint32_t irq) {
    ioapic_mask_irq(irq);
}

void irq_unmask(uint32_t irq) {
    ioapic_unmask_irq(irq);
}