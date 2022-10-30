#pragma once

#include <kernel/arch/paging.h>

void *acpi_find_table(void *rsdp_addr, const char *name);