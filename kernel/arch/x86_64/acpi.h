#ifndef ESTELLA_ARCH_X86_64_ACPI_H
#define ESTELLA_ARCH_X86_64_ACPI_H

#include <stdint.h>

struct rsdp2 {
    char signature[8];
    uint8_t checksum;
    char oemid[6];
    uint8_t revision;
    uint32_t rsdt_address;

    uint32_t length;
    uint64_t xsdt_address;
    uint8_t extended_checksum;
    uint8_t reserved[3];
} __attribute__((packed));

struct acpi_sdt_header {
    char signature[4];
    uint32_t length;
    uint8_t revision;
    uint8_t checksum;
    char oemid[6];
    char oem_table_id[8];
    uint32_t oem_revision;
    uint32_t creator_id;
    uint32_t creator_revision;
} __attribute__((packed));

struct xsdt {
    struct acpi_sdt_header header;
    uint64_t entries[];
} __attribute__((packed));

struct madt {
    struct acpi_sdt_header header;
    uint32_t lapic_address;
    uint32_t flags;
} __attribute__((packed));


struct xsdt* acpi_get_xsdt(void* rsdp_ptr);
struct acpi_sdt_header* acpi_find_table(struct xsdt* xsdt, const char* signature);
struct madt* acpi_get_madt(void* rsdp_ptr);

#endif