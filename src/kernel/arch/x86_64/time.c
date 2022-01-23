#include <kernel/arch/time.h>

#include <stdio.h>

#include <kernel/tasking.h>

#include <kernel/arch/cpu.h>
#include <kernel/arch/pit.h>

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

uint64_t boot_time = 0;
uint64_t tsc_freq = 0;
uint64_t tsc_basis = 0;

static uint64_t secs_to_year(uint64_t year) {
    uint64_t leap_years = (year - 1968)/4;
    return ((year - 1970) * 365 + leap_years) * 86400;
}

#pragma GCC diagnostic ignored "-Wimplicit-fallthrough"
#pragma GCC diagnostic push
static uint64_t secs_to_month(uint16_t month, uint16_t year) {
    uint64_t days = 0;

    switch (month)
    {
    case 12:
        days += 30;
    case 11:
        days += 31;
    case 10:
        days += 30;
    case 9:
        days += 31;
    case 8:
        days += 31;
    case 7:
        days += 30;
    case 6:
        days += 31;
    case 5:
        days += 30;
    case 4:
        days += 31;
    case 3:
        if ((year % 4 == 0) && ((year % 100 != 0) || (year % 400 == 0))) {
            days += 29;
        } else {
            days += 28;
        }
    case 2:
        days += 31;
    default:
        break;
    }

    return days * 86400;
}
#pragma GCC diagnostic pop

static uint8_t cmos_read_register(uint8_t reg) {
    outb(CMOS_CONTROL_PORT, reg);
    return inb(CMOS_REGISTER_PORT);
}

static uint8_t bcd_to_binary(uint8_t bcd) {
    return ((bcd & 0xF0) >> 1) + ((bcd & 0xF0) >> 3) + (bcd & 0x0F);
}

uint64_t read_cmos(void) {
    //uint8_t status_a = cmos_read_register(CMOS_STATUS_A_REGISTER);
    uint8_t status_b = cmos_read_register(CMOS_STATUS_B_REGISTER);

    uint16_t second = cmos_read_register(CMOS_SECONDS_REGISTER);
    uint16_t minute = cmos_read_register(CMOS_MINUTES_REGISTER);
    uint16_t hour = cmos_read_register(CMOS_HOURS_REGISTER);
    uint16_t day = cmos_read_register(CMOS_DAYS_REGISTER);
    uint16_t month = cmos_read_register(CMOS_MONTHS_REGISTER);
    uint16_t year = cmos_read_register(CMOS_YEARS_REGISTER);
    uint16_t century = cmos_read_register(CMOS_CENTURY_REGISTER);

    if (!(status_b & CMOS_STATUS_B_BINARY_FLAG)) {
        second = bcd_to_binary(second);
        minute = bcd_to_binary(minute);
        hour = bcd_to_binary(hour & 0x7F) | (hour & 0x80);
        day = bcd_to_binary(day);
        month = bcd_to_binary(month);
        year = bcd_to_binary(year);
        century = bcd_to_binary(century);
    }

    if (!(status_b & CMOS_STATUS_B_24HR_FLAG)) {
        hour = hour & (1 << 7) ? (hour & 0x7F) + 12 : hour;
        if (hour & (1 << 7)) {
            hour &= 0x7f;
            hour += 12;
            if (hour == 24) {
                hour = 12;
            }
        } else {
            if (hour == 12) {
                hour = 0;
            }
        }
    }

    uint64_t unix_time = secs_to_year(century * 100 + year) + secs_to_month(month, century * 100 + year)
         + (day - 1) * 86400 + hour * 3600 + minute * 60 + second;

    return unix_time;
}

uint64_t read_tsc(void) {
    uint32_t high, low;
    asm volatile ("rdtsc" : "=a"(low), "=d"(high));
    return ((uint64_t)high << 32) + low;
}

void clock_init(void) {
    pit_init(100);
    uint64_t before, after;
    uint64_t pit_ticks = pit_get_ticks();
    while(pit_ticks == pit_get_ticks());
    before = read_tsc();
    pit_ticks = pit_get_ticks();
    while(pit_ticks == pit_get_ticks());
    after = read_tsc();
    tsc_freq = (after - before) * 100;
    pit_stop();

    tsc_basis = after;
    boot_time = read_cmos();

    printf("Booted at: %lu\n", boot_time);
    printf("TSC basis: %lu\n", tsc_basis);
    printf("TSC Frequency: %lu hz\n", tsc_freq);
}

/**
 * @brief Returns time from boot in microseconds.
 */
uint64_t get_time_from_boot(void) {
    return (read_tsc() - tsc_basis) / (tsc_freq / 1000000);
}

void update_clock(void) {
    wakeup_sleepers();
}