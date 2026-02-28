#ifndef ESTELLA_ARCH_X86_64_CPU_IO_H
#define ESTELLA_ARCH_X86_64_CPU_IO_H

#include <stdint.h>

uint8_t inb(uint16_t port);
void outb(uint8_t value, uint16_t port);

uint16_t inmw(uint16_t port);
void outw(uint16_t value, uint16_t port);

uint32_t inl(uint16_t port);
void outl(uint32_t value, uint16_t port);

#endif