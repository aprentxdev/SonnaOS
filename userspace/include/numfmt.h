#pragma once

#include <stddef.h>

size_t utoa_base(unsigned long value, char *buf, int base);
size_t itoa_signed(long value, char *buf);