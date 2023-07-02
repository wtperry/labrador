#pragma once

#define GDT_NULL_DESC           0x00
#define GDT_KERNEL_CODE_DESC    0x08
#define GDT_KERNEL_DATA_DESC    0x10
#define GDT_USER_CODE_DESC      0x18
#define GDT_USER_DATA_DESC      0x20

void gdt_init();