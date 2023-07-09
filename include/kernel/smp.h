#pragma once

#include <kernel/mem.h>

#include <stdint.h>

#define MAX_CORES 1

typedef struct {
    union paging_entry_t *current_pml4;
} processor_data_t;

extern processor_data_t processor_local_data[MAX_CORES];
static processor_data_t __seg_gs * const this_core = 0;

void set_gs_base(uintptr_t gs_base);