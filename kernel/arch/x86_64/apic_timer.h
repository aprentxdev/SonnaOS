#ifndef ESTELLA_ARCH_X86_64_APIC_TIMER_H
#define ESTELLA_ARCH_X86_64_APIC_TIMER_H

#include <stdint.h>
#include <stdbool.h>

extern volatile bool lapic_timer_needed;

void apic_timer_init(void);
void lapic_timer_handler(void);

#endif