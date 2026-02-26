#ifndef ESTELLA_ARCH_X86_64_LAPIC_H
#define ESTELLA_ARCH_X86_64_LAPIC_H

#include <stdint.h>
#include <stdbool.h>

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

#define LAPIC_SVR_ENABLE (1U << 8)
#define LAPIC_MASKED (1U << 16)
#define LAPIC_MODE_PERIODIC (1U << 17)
#define LAPIC_LVT_TIMER_TSC_DEADLINE (1U << 18)

#define LAPIC_TIMER_VECTOR 0x20
#define LAPIC_ERROR_VECTOR 0xFE
#define LAPIC_SPURIOUS_VECTOR 0xFF

#define IA32_APIC_BASE_MSR  0x1B
#define IA32_APIC_BASE_ENABLE (1U << 11)
#define IA32_APIC_BASE_X2APIC (1U << 10)
#define IA32_TSC_DEADLINE 0x6E0

extern volatile uint64_t lapic_ticks;
extern uint64_t lapic_phys;
extern uint64_t lapic_va;
extern bool x2apic_enabled;

uint32_t lapic_read(uint32_t reg);
void lapic_write(uint32_t reg, uint32_t value);

void lapic_init(void);
void lapic_eoi(void);

#endif