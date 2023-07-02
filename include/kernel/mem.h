#pragma once

#include <stdint.h>

#define PAGE_SIZE 4096

typedef uintptr_t paddr_t;

void mem_init();
void mem_free_phys_page(paddr_t page_addr);
paddr_t mem_get_phys_page();