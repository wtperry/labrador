#include "time.h"
#include "../ports/ports.h"
#include <kernel/hal.h>
#include <stdbool.h>

#define CMOS_CONTROL_PORT           0x70
#define CMOS_REGISTER_PORT          0x71

#define CMOS_SECONDS_REGISTER       0x00
#define CMOS_MINUTES_REGISTER       0x02
#define CMOS_HOURS_REGISTER         0x04
#define CMOS_DAYS_REGISTER          0x07
#define CMOS_MONTHS_REGISTER        0x08
#define CMOS_YEARS_REGISTER         0x09
#define CMOS_CENTURY_REGISTER       0x32

#define CMOS_STATUS_A_REGISTER      0x0A
#define CMOS_STATUS_B_REGISTER      0x0B

#define CMOS_STATUS_B_24HR_FLAG     (1 << 1)
#define CMOS_STATUS_B_BINARY_FLAG   (1 << 2)

uint8_t cmos_read_register(uint8_t reg) {
    outb(CMOS_CONTROL_PORT, reg);
    return inb(CMOS_REGISTER_PORT);
}

uint8_t bcd_to_binary(uint8_t bcd) {
    return ((bcd & 0xF0) >> 1) + ((bcd & 0xF0) >> 3) + (bcd & 0x0F);
}

struct date_time get_cmos_time(void) {
    struct date_time curr_time;

    uint8_t status_a = cmos_read_register(CMOS_STATUS_A_REGISTER);
    uint8_t status_b = cmos_read_register(CMOS_STATUS_B_REGISTER);

    bool is_binary = status_b & CMOS_STATUS_B_BINARY_FLAG;
    bool is_24_hr = status_b & CMOS_STATUS_B_24HR_FLAG;

    curr_time.second = cmos_read_register(CMOS_SECONDS_REGISTER);
    curr_time.minute = cmos_read_register(CMOS_MINUTES_REGISTER);
    curr_time.hour = cmos_read_register(CMOS_HOURS_REGISTER);
    curr_time.day = cmos_read_register(CMOS_DAYS_REGISTER);
    curr_time.month = cmos_read_register(CMOS_MONTHS_REGISTER);
    curr_time.year = cmos_read_register(CMOS_YEARS_REGISTER);
    curr_time.century = cmos_read_register(CMOS_CENTURY_REGISTER);

    if (!is_binary) {
        curr_time.second = bcd_to_binary(curr_time.second);
        curr_time.minute = bcd_to_binary(curr_time.minute);
        curr_time.hour = bcd_to_binary(curr_time.hour);
        curr_time.day = bcd_to_binary(curr_time.day);
        curr_time.month = bcd_to_binary(curr_time.month);
        curr_time.year = bcd_to_binary(curr_time.year);
        curr_time.century = bcd_to_binary(curr_time.century);
    }

    if (!is_24_hr) {
        curr_time.hour = curr_time.hour & (1 << 7) ? (curr_time.hour & 0x7F) + 12 : curr_time.hour;
    }

    return curr_time;
}