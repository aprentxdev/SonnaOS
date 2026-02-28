#ifndef ESTELLA_ARCH_X86_64_TIME_HPET_H
#define ESTELLA_ARCH_X86_64_TIME_HPET_H

#include <stdint.h>

extern uint64_t hpet_va;
extern uint64_t hpet_frequency_hz;

void hpet_init(void *rsdp_ptr);
uint64_t hpet_read(uint64_t offset);
void hpet_write(uint64_t offset, uint64_t value);

#endif