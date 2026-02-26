#include <arch/x86_64/apic.h>
#include <arch/x86_64/lapic.h>
#include <arch/x86_64/ioapic.h>
#include <arch/x86_64/apic_timer.h>
#include <arch/x86_64/tsc.h>
#include <arch/x86_64/acpi.h>
#include <drivers/serial.h>
#include <limine.h>

extern volatile struct limine_rsdp_request rsdp_request;

void apic_init(void) {
    void *rsdp_ptr = rsdp_request.response->address;
    struct madt* madt = acpi_get_madt(rsdp_ptr);
    if (!madt) {
        return;
    }

    lapic_init();
    tsc_init();
    apic_timer_init();
    ioapic_init_all(madt);

    serial_puts("APIC fully initialized\n");
}