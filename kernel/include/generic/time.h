#ifndef ESTELLA_GENERIC_TIME_H
#define ESTELLA_GENERIC_TIME_H

#include <stdint.h>

void time_init(void);
char* time_get_current(void); // YYYY/MM/DD HH:MM:SS (UTC)
uint64_t time_get_timestamp(void); // UNIX timestamp

#endif