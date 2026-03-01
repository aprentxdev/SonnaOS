#ifndef ESTELLA_ARCH_X86_64_USERMODE_USERMODE_H
#define ESTELLA_ARCH_X86_64_USERMODE_USERMODE_H

#include <stdint.h>
#include <stddef.h>

void enter_usermode(void *elf_data, size_t elf_size);

#endif