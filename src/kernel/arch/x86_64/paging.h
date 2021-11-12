#pragma once

#define PAGE_SIZE           4096  //4 KiB
#define PHYSICAL_MAP_BASE   0xFFFF800000000000 //all physical memory mapped to bottom of upper half of memory

#define PHYS_TO_VIRT(x)     (void*)((uint64_t)x + PHYSICAL_MAP_BASE)

#define PAGE_ADDR_MASK 0x000ffffffffff000

#define PAGE_PRESENT (1 << 0)
#define PAGE_WRITEABLE (1 << 1)