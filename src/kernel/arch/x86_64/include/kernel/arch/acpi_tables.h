#pragma once

#include <stdint.h>

struct rsdp {
    char signature[8];
    uint8_t checksum;
    char oem_id[6];
    uint8_t revision;
    uint32_t rsdt_addr;
} __attribute__((packed));

struct rsdp2 {
    char signature[8];
    uint8_t checksum;
    char oem_id[6];
    uint8_t revision;
    uint32_t rsdt_addr;
    uint32_t length;
    uint64_t xsdt_addr;
    uint8_t extended_checksum;
    uint8_t reserved[3];
} __attribute__((packed));

struct sdt_hdr {
    char signature[4];
    uint32_t length;
    uint8_t revision;
    uint8_t checksum;
    char oem_id[6];
    char oem_table_id[8];
    uint32_t oem_revision;
    uint32_t creator_id;
    uint32_t creator_revision;
} __attribute__((packed));

struct xsdt {
    struct sdt_hdr hdr;
    uint64_t sdt_ptrs[];
} __attribute__((packed));

struct madt {
    struct sdt_hdr hdr;
    uint32_t lapic_addr;
    uint32_t flags;
    uint8_t entries[];
} __attribute__((packed));

struct madt_entry {
    uint8_t type;
    uint8_t length;
} __attribute__((packed));

struct madt_entry_0 {
    struct madt_entry hdr;
    uint8_t proc_id;
    uint8_t apic_id;
    uint32_t flags;
} __attribute__((packed));

struct madt_entry_1 {
    struct madt_entry hdr;
    uint8_t ioapic_id;
    uint8_t reserved;
    uint32_t ioapic_addr;
    uint32_t int_base;
} __attribute__((packed));

struct madt_entry_2 {
    struct madt_entry hdr;
    uint8_t bus;
    uint8_t irq;
    uint32_t int_num;
    uint16_t flags;
} __attribute__((packed));

struct address_struct {
    uint8_t address_space_id;
    uint8_t register_bit_width;
    uint8_t register_bit_offset;
    uint8_t reserved;
    uint64_t address;
} __attribute__((packed));

struct hpet {
    struct sdt_hdr hdr;
    uint8_t hardware_rev_id;
    uint8_t comparator_count:5;
    uint8_t counter_size:1;
    uint8_t reserved:1;
    uint8_t legacy_replacement:1;
    uint16_t pci_vendor_id;
    struct address_struct address;
    uint8_t hpet_number;
    uint16_t minimum_tick;
    uint8_t page_protection;
} __attribute__((packed));

struct mcfg_entry {
    uint64_t base_addr;
    uint16_t segment_group;
    uint8_t start_bus;
    uint8_t end_bus;
    uint32_t reserved;
} __attribute__((packed));

struct mcfg {
    struct sdt_hdr hdr;
    uint64_t reserved;
    struct mcfg_entry entries[];
} __attribute__((packed));