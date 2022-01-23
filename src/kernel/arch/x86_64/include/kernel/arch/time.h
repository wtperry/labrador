#pragma once

#include <stdint.h>

extern uint64_t boot_time;
extern uint64_t tsc_freq;
extern uint64_t tsc_basis;

void clock_init(void);
uint64_t read_tsc(void);
uint64_t get_time_from_boot(void);
void update_clock(void);