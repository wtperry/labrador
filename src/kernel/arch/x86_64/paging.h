#pragma once

#define PAGE_SIZE           4096  //4 KiB
#define PHYSICAL_MAP_BASE   0xFFFF800000000000 //all physical memory mapped to bottom of upper half of memory

#define PHYS_TO_VIRT(x)     ((uint64_t)x + PHYSICAL_MAP_BASE)