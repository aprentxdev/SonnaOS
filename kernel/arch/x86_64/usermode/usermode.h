#ifndef ESTELLA_ARCH_X86_64_USERMODE_USERMODE_H
#define ESTELLA_ARCH_X86_64_USERMODE_USERMODE_H

#include <stdint.h>
#include <stddef.h>
#include <arch/x86_64/usermode/scheduler.h>

void runElf_ring3(void *elf_data, size_t elf_size);
void task_enter(task_t *task);

#endif