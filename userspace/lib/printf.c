#include <stdarg.h>
#include <stddef.h>

#include <syscalls.h>
#include <numfmt.h>
#include <string.h>
#include <printf.h>

void printf(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);

    char buf[128];
    while (*fmt) {
        if (*fmt == '%') {
            fmt++;
            int len = 0;
            switch (*fmt) {
                case 's': {
                    const char *s = va_arg(args, const char *);
                    write(1, s, strlen(s));
                    break;
                }
                case 'd': {
                    int d = va_arg(args, int);
                    len = itoa_signed(d, buf);
                    write(1, buf, len);
                    break;
                }
                case 'x': {
                    unsigned int x = va_arg(args, unsigned int);
                    len = utoa_base(x, buf, 16);
                    write(1, buf, len);
                    break;
                }
                case 'l': {
                    fmt++;
                    if (*fmt == 'l') {
                        fmt++;
                        if (*fmt == 'd') {
                            long long d = va_arg(args, long long);
                            len = itoa_signed((long)d, buf);
                            write(1, buf, len);
                        } else {
                            write(1, "?", 1);
                        }
                    } else if (*fmt == 'd') {
                        long d = va_arg(args, long);
                        len = itoa_signed(d, buf);
                        write(1, buf, len);
                    } else {
                        write(1, "?", 1);
                    }
                    break;
                }
                case '%':
                    write(1, "%", 1);
                    break;
                default:
                    write(1, "?", 1);
            }
        } else {
            write(1, fmt, 1);
        }
        fmt++;
    }

    va_end(args);
}