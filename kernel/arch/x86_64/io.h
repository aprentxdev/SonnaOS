#ifndef ESTELLA_ARCH_X86_64_IO_H
#define ESTELLA_ARCH_X86_64_IO_H

#include <stdint.h>

static inline uint8_t inb(uint16_t port)
{
    uint8_t value;
    __asm__ volatile ("inb %1, %0" : "=a" (value) : "Nd" (port));
    return value;
}

static inline void outb(uint8_t value, uint16_t port)
{
    __asm__ volatile ("outb %0, %1" : : "a" (value), "Nd" (port));
}

static inline uint16_t inw(uint16_t port)
{
    uint16_t value;
    __asm__ volatile ("inw %1, %0" : "=a" (value) : "Nd" (port));
    return value;
}

static inline void outw(uint16_t value, uint16_t port)
{
    __asm__ volatile ("outw %0, %1" : : "a" (value), "Nd" (port));
}

static inline uint32_t inl(uint16_t port)
{
    uint32_t value;
    __asm__ volatile ("inl %1, %0" : "=a" (value) : "Nd" (port));
    return value;
}

static inline void outl(uint32_t value, uint16_t port)
{
    __asm__ volatile ("outl %0, %1" : : "a" (value), "Nd" (port));
}

#endif