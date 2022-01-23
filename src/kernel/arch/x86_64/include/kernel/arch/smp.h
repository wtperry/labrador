#pragma once

#include <stdint.h>

#include <kernel/tasking.h>

#define MAX_CORES 1

typedef struct processor_data {
    volatile thread_t *current_thread;
} processor_data_t;

extern processor_data_t processor_local_data[MAX_CORES];
static processor_data_t __seg_gs * const this_core = 0;

void set_gs_base(uint64_t gs_base);