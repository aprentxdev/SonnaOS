#ifndef ESTELLA_ARCH_X86_64_TIME_TIME_H
#define ESTELLA_TIME_X86_64_TIME_TIME_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

void time_init(void);

// YYYY/MM/DD HH:MM:SS
char* time_get_current(void);

// UNIX timestamp
uint64_t time_get_timestamp(void);

#endif