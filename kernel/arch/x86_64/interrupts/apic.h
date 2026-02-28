#ifndef ESTELLA_ARCH_X86_64_INTERRUPTS_APIC_H
#define ESTELLA_ARCH_X86_64_INTERRUPTS_APIC_H

#include <arch/x86_64/interrupts/lapic.h>
#include <arch/x86_64/interrupts/ioapic.h>
#include <arch/x86_64/interrupts/apictimer.h>
#include <arch/x86_64/time/tsc.h>

void apic_init(void);

#endif