#ifndef ESTELLA_DRIVERS_KEYBOARD_H
#define ESTELLA_DRIVERS_KEYBOARD_H

#include <stdint.h>
#include <stdbool.h>

void keyboard_init(void);
bool keyboard_has_data(void);
uint8_t keyboard_get_scancode(void);
char keyboard_get_char(void);

#endif