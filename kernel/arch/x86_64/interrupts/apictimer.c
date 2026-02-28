#include <arch/x86_64/interrupts/apictimer.h>
#include <arch/x86_64/interrupts/lapic.h>
#include <arch/x86_64/time/tsc.h>
#include <arch/x86_64/cpu/msr.h>
#include <arch/x86_64/cpu/cpuid.h>
#include <drivers/serial.h>

volatile bool lapic_timer_needed = false;

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

void apic_timer_init(void) {
    uint32_t eax, ebx, ecx, edx;
    cpuid(1, &eax, &ebx, &ecx, &edx);

    bool tsc_deadline_supported = (ecx & (1U << 24)) != 0;
    bool tsc_invariant = tsc_is_invariant();
    bool use_tsc_deadline = tsc_deadline_supported && (tsc_frequency_hz != 0) && tsc_invariant;

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
    serial_puts("LAPIC timer initialized\n");
}