#include <kernel/arch/pci.h>

#include <libk/stdio.h>

#include <kernel/arch/acpi.h>
#include <kernel/arch/acpi_tables.h>

struct pci_hdr_common {
    uint16_t vendor_id;
    uint16_t device_id;
    uint16_t command;
    uint16_t status;
    uint8_t revision_id;
    uint8_t prog_if;
    uint8_t subclass;
    uint8_t class;
    uint8_t cache_line_size;
    uint8_t latency_timer;
    uint8_t header_type;
    uint8_t bist;
}__attribute__((packed));

void pci_enumerate_function(void* dev_base_addr, int bus, int device, int function) {
    struct pci_hdr_common *hdr = (struct pci_hdr_common *)(((uint64_t)dev_base_addr) + (function << 12));

    if (hdr->vendor_id == 0xFFFF) {
        // function does not exist
        return;
    }

    printf("Bus: %u, Device: %u, Function %u, Vendor ID: %lx, Device ID: %lx, Class: %x Sub Class: %x\n", bus, device, function, hdr->vendor_id, hdr->device_id, hdr->class, hdr->subclass);
}

void pci_enumerate_device(void *bus_base_addr, int bus, int device) {
    void *dev_base_addr = (void *)(((uint64_t)bus_base_addr) + (device << 15));
    struct pci_hdr_common *hdr = (struct pci_hdr_common *)dev_base_addr;

    if (hdr->vendor_id == 0xFFFF) {
        // device does not exist
        return;
    }

    pci_enumerate_function(dev_base_addr, bus, device, 0);

    if ((hdr->header_type & 0x80) != 0) {
        // device has multiple functions
        for (int function = 1; function < 8; function++) {
            pci_enumerate_function(dev_base_addr, bus, device, function);
        }
    }
}

void pci_enumerate_bus(void *base_addr, int bus, int start_bus) {
    void *bus_base_addr = (void *)(((uint64_t)base_addr) + ((bus - start_bus) << 20));

    for (int device = 0; device < 32; device++) {
        pci_enumerate_device(bus_base_addr, bus, device);
    }
}

void pci_enumerate(void *rsdp_addr) {
    struct mcfg *mcfg = acpi_find_table(rsdp_addr, "MCFG");

    int mcfg_entries = (mcfg->hdr.length - sizeof(mcfg->hdr)) / sizeof(struct mcfg_entry);

    for (int i = 0; i < mcfg_entries; i++) {
        printf("MMIO Addr: %llx, Seg Num: %u, Start: %u, End: %u\n", mcfg->entries[i].base_addr, mcfg->entries[i].segment_group, mcfg->entries[i].start_bus, mcfg->entries[i].end_bus);
        int start_bus = mcfg->entries[i].start_bus;
        int end_bus = mcfg->entries[i].end_bus;

        for (int bus = start_bus; bus <= end_bus; bus++) {
            pci_enumerate_bus(PHYS_TO_VIRT(mcfg->entries[i].base_addr), bus, start_bus);
        }
    }
}