#include <kernel/smp.h>

#include <kernel/x86.h>

processor_data_t processor_local_data[MAX_CORES];

#define GSBASE_MSR 0xC0000101

void set_gs_base(uintptr_t gs_base) {
    load_msr(GSBASE_MSR, gs_base);
}