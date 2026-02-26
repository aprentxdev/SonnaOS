#ifndef ESTELLA_ARCH_X86_64_APIC_H
#define ESTELLA_ARCH_X86_64_APIC_H

#include <arch/x86_64/lapic.h>
#include <arch/x86_64/ioapic.h>
#include <arch/x86_64/apic_timer.h>
#include <arch/x86_64/tsc.h>

void apic_init(void);

#endif