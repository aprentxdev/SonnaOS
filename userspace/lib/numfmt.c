#include <numfmt.h>

size_t utoa_base(unsigned long value, char *buf, int base) {
    const char digits[] = "0123456789abcdef";
    char tmp[32];
    int i = 0;

    if (base < 2 || base > 16) {
        buf[0] = '\0';
        return 0;
    }

    if (value == 0) {
        buf[0] = '0';
        return 1;
    }

    while (value > 0) {
        tmp[i++] = digits[value % base];
        value /= base;
    }

    for (int j = 0; j < i; j++) {
        buf[j] = tmp[i - 1 - j];
    }

    return i;
}

size_t itoa_signed(long value, char *buf) {
    size_t len = 0;
    unsigned long abs_val;

    if (value < 0) {
        buf[0] = '-';
        abs_val = (unsigned long)(-value);
        len = 1 + utoa_base(abs_val, buf + 1, 10);
    } else {
        abs_val = (unsigned long)value;
        len = utoa_base(abs_val, buf, 10);
    }

    return len;
}