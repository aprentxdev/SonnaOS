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

#define MADT_ENTRY_TYPE_LOCAL_APIC           0
#define MADT_ENTRY_TYPE_IO_APIC              1
#define MADT_ENTRY_TYPE_INTERRUPT_OVERRIDE   2
#define MADT_ENTRY_TYPE_LOCAL_APIC_NMI       4
#define MADT_ENTRY_TYPE_LOCAL_X2APIC         9

struct madt_entry_header {
    uint8_t  type;
    uint8_t  length;
} __attribute__((packed));

struct madt_local_apic {
    struct madt_entry_header header;
    uint8_t  processor_id;
    uint8_t  apic_id;
    uint32_t flags;
} __attribute__((packed));

struct madt_ioapic {
    struct madt_entry_header header;
    uint8_t  ioapic_id;
    uint8_t  reserved;
    uint32_t ioapic_address;
    uint32_t global_system_interrupt_base;
} __attribute__((packed));

struct madt_iso {
    struct madt_entry_header header;
    uint8_t  bus_source;
    uint8_t  irq_source;
    uint32_t global_system_interrupt;
    uint16_t flags;
} __attribute__((packed));

#define ISO_POLARITY_MASK       0x0003
#define ISO_POLARITY_DEFAULT    0x0000
#define ISO_POLARITY_HIGH       0x0001
#define ISO_POLARITY_LOW        0x0003

#define ISO_TRIGGER_MASK        0x000C
#define ISO_TRIGGER_DEFAULT     0x0000
#define ISO_TRIGGER_EDGE        0x0004
#define ISO_TRIGGER_LEVEL       0x000C

struct xsdt* acpi_get_xsdt(void* rsdp_ptr);
struct acpi_sdt_header* acpi_find_table(struct xsdt* xsdt, const char* signature);
struct madt* acpi_get_madt(void* rsdp_ptr);

extern volatile struct limine_rsdp_request rsdp_request;

#endif