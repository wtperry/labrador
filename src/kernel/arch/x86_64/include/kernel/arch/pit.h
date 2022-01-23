#pragma once

#include <stdint.h>

void pit_init(int freq);
uint64_t pit_get_ticks(void);
void pit_stop(void);