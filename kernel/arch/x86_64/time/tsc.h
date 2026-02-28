#ifndef ESTELLA_ARCH_X86_64_TIME_TSC_H
#define ESTELLA_ARCH_X86_64_TIME_TSC_H

#include <stdint.h>
#include <stdbool.h>

extern uint64_t tsc_frequency_hz;
extern uint64_t tsc_ticks_per_10ms;

void tsc_init(void);
uint64_t rdtsc(void);
bool tsc_is_invariant(void);

#endif