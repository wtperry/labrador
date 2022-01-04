#ifndef TIME_H
#define TIME_H

#include <stdint.h>

struct date_time
{
    uint8_t second;
    uint8_t minute;
    uint8_t hour;
    uint8_t day;
    uint8_t month;
    uint8_t year;
    uint8_t century;
};

struct date_time get_cmos_time(void);

#endif