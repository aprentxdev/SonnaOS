#include <arch/x86_64/time/tsc.h>
#include <arch/x86_64/acpi/acpi.h>
#include <arch/x86_64/cpu/cpuid.h>
#include <arch/x86_64/time/hpet.h>
#include <drivers/serial.h>
#include <klib/string.h>
#include <limine.h>

extern volatile struct limine_rsdp_request rsdp_request;

uint64_t tsc_frequency_hz = 0;
uint64_t tsc_ticks_per_10ms = 0;

uint64_t rdtsc(void) {
    uint32_t low, high;
    asm volatile("rdtsc" : "=a"(low), "=d"(high));
    return ((uint64_t)high << 32) | low;
}

bool tsc_is_invariant(void) {
    uint32_t eax, ebx, ecx, edx;
    bool tsc_invariant = false;

    cpuid(0x80000000, &eax, &ebx, &ecx, &edx);
    uint32_t max_ext_leaf = eax;

    if (max_ext_leaf >= 0x80000007) {
        cpuid(0x80000007, &eax, &ebx, &ecx, &edx);
        tsc_invariant = (edx & (1 << 8)) != 0;
    }

    if (!tsc_invariant)
        serial_puts("TSC not invariant\n");

    return tsc_invariant;
}

static uint64_t tsc_calibrate_cpuid(void) {
    uint32_t eax, ebx, ecx, edx;
    cpuid(0, &eax, &ebx, &ecx, &edx);
    uint32_t max_leaf = eax;

    if (max_leaf >= 0x15) {
        cpuid(0x15, &eax, &ebx, &ecx, &edx);

        if (eax && ebx && ecx) {
            return ((uint64_t)ecx * ebx) / eax;
        }
    }

    if (max_leaf >= 0x16) {
        cpuid(0x16, &eax, &ebx, &ecx, &edx);

        if (eax)
            return (uint64_t)eax * 1000000ULL;
    }

    return 0;
}

static uint64_t tsc_calibrate_hpet(void) {
    serial_puts("TSC calibration via CPUID failed, trying via HPET\n");

    hpet_init(rsdp_request.response->address);
    if (hpet_frequency_hz == 0) {
        serial_puts("HPET init failed, no TSC calibration\n");
        return 0;
    }

    const uint64_t calibration_duration_ns = 100000000ULL;
    const uint64_t ticks_to_wait = (hpet_frequency_hz * calibration_duration_ns) / 1000000000ULL;

    uint64_t hpet_start = hpet_read(HPET_MAIN_COUNTER);
    uint64_t tsc_start = rdtsc();

    while (hpet_read(HPET_MAIN_COUNTER) - hpet_start < ticks_to_wait) {
        asm("pause");
    }

    uint64_t tsc_end = rdtsc();
    uint64_t hpet_end = hpet_read(HPET_MAIN_COUNTER);

    uint64_t hpet_delta = hpet_end - hpet_start;
    uint64_t tsc_delta = tsc_end - tsc_start;

    return (tsc_delta * hpet_frequency_hz) / hpet_delta;
}

uint64_t tsc_calibrate(void) {
    uint64_t freq = tsc_calibrate_cpuid();

    if (freq) {
        serial_puts("TSC calibrated via CPUID\n");
    } else {
        freq = tsc_calibrate_hpet();
        if (freq) {
            serial_puts("TSC calibrated via HPET: ");
            char buf[32];
            u64_to_dec(freq, buf);
            serial_puts(buf);
            serial_puts(" Hz\n");
        }
    }

    return freq;
}

void tsc_init(void) {
    tsc_frequency_hz = tsc_calibrate();

    if (!tsc_frequency_hz) {
        serial_puts("TSC initialization failed\n");
        return;
    }

    tsc_ticks_per_10ms = tsc_frequency_hz / 100;
    serial_puts("TSC initialized\n");
    return;
}