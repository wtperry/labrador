#include <kernel/arch/acpi.h>

#include <libk/string.h>

#include <kernel/arch/acpi_tables.h>
#include <kernel/arch/paging.h>

void *acpi_find_table(void *rsdp_addr, const char *name) {
    struct rsdp *rsdp = (struct rsdp *)rsdp_addr;

    if (rsdp->revision == 2) {
        struct rsdp2 *rsdp2 = (struct rsdp2 *)rsdp;

        struct xsdt *xsdt = (struct xsdt *)PHYS_TO_VIRT(rsdp2->xsdt_addr);

        int xsdt_entries = (xsdt->hdr.length - sizeof(xsdt->hdr)) / 8;

        for (int i = 0; i < xsdt_entries; i++) {
            struct sdt_hdr *hdr = (struct sdt_hdr *)PHYS_TO_VIRT(xsdt->sdt_ptrs[i]);

            if (!strncmp(hdr->signature, name, 4)) {
                return (void *)hdr;
            }
        }
    }

    return NULL;
}