#ifndef ESTELLA_STOPWATCH_H
#define ESTELLA_STOPWATCH_H

#include <stdint.h>
#include <stdbool.h>

void stopwatch_init(void);
void stopwatch_update(uint64_t current_tsc, uint64_t tsc_per_sec);
bool stopwatch_is_running(void);
void stopwatch_toggle(void);
void stopwatch_reset(void);

uint64_t stopwatch_get_elapsed_seconds(void);

#endif