#ifndef ESTELLA_ARCH_X86_64_CPU_CPUID_H
#define ESTELLA_ARCH_X86_64_CPU_CPUID_H

#include <stdint.h>

static inline void cpuid(uint32_t leaf, uint32_t *eax, uint32_t *ebx, uint32_t *ecx, uint32_t *edx) {
    asm volatile("cpuid"
                 : "=a"(*eax), "=b"(*ebx), "=c"(*ecx), "=d"(*edx)
                 : "a"(leaf), "c"(0));
}

#endif