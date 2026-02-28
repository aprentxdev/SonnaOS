#include <stdint.h>
#include <klib/string.h>
#include <arch/x86_64/time/time.h>
#include <generic/time.h>
#include <arch/x86_64/time/tsc.h>

#include <limine.h>

extern volatile struct limine_date_at_boot_request date_at_boot_request;

static uint64_t boot_timestamp = 0;
static uint64_t boot_tsc = 0;

void time_init(void) {
    if (date_at_boot_request.response && date_at_boot_request.response->timestamp >= 0) {
        boot_timestamp = date_at_boot_request.response->timestamp;
    } else {
        boot_timestamp = 0;
    }

    boot_tsc = rdtsc();
}

uint64_t time_get_timestamp(void) {
    uint64_t delta_ticks = rdtsc() - boot_tsc;
    uint64_t delta_sec = delta_ticks / tsc_frequency_hz;
    return boot_timestamp + delta_sec;
}

char* time_get_current(void) {
    static char current_datetime[20];
    uint64_t ts = time_get_timestamp();

    uint64_t days = ts / 86400;
    uint64_t secs_in_day = ts % 86400;
    uint64_t hour = secs_in_day / 3600;
    uint64_t min = (secs_in_day % 3600) / 60;
    uint64_t sec = secs_in_day % 60;

    uint64_t year = 1970;
    while (true) {
        bool leap = (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0));
        uint64_t days_in_year = leap ? 366 : 365;
        if (days >= days_in_year) {
            days -= days_in_year;
            year++;
        } else {
            break;
        }
    }

    static const uint8_t month_days[12] = {31,28,31,30,31,30,31,31,30,31,30,31};
    uint64_t month = 0;
    while (true) {
        uint64_t dim = month_days[month];
        if (month == 1) {
            bool leap = (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0));
            if (leap) dim = 29;
        }

        if (days >= dim) {
            days -= dim;
            month++;
        } else {
            break;
        }
    }

    uint64_t day = days + 1;

    current_datetime[0] = '0' + (year / 1000 % 10);
    current_datetime[1] = '0' + (year / 100 % 10);
    current_datetime[2] = '0' + (year / 10 % 10);
    current_datetime[3] = '0' + (year % 10);
    current_datetime[4] = '/';
    current_datetime[5] = '0' + (month + 1) / 10;
    current_datetime[6] = '0' + (month + 1) % 10;
    current_datetime[7] = '/';
    current_datetime[8] = '0' + (day) / 10;
    current_datetime[9] = '0' + (day) % 10;
    current_datetime[10] = ' ';
    current_datetime[11] = '0' + hour / 10;
    current_datetime[12] = '0' + hour % 10;
    current_datetime[13] = ':';
    current_datetime[14] = '0' + min / 10;
    current_datetime[15] = '0' + min % 10;
    current_datetime[16] = ':';
    current_datetime[17] = '0' + sec / 10;
    current_datetime[18] = '0' + sec % 10;
    current_datetime[19] = '\0';

    return current_datetime;
}