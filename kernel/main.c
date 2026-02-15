#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include <limine.h>

#include <klib/memory.h>
#include <klib/string.h>
#include <arch/x86_64/gdt.h>
#include <arch/x86_64/idt.h>
#include <arch/x86_64/acpi.h>
#include <arch/x86_64/apic.h>
#include <drivers/font.h>
#include <drivers/fbtext.h>
#include <drivers/serial.h>
#include <drivers/keyboard.h>
#include <mm/pmm.h>
#include <mm/vmm.h>
#include <colors.h>

#define ESTELLA_VERSION "v0.Estella.6.0"

__attribute__((used, section(".limine_requests")))
static volatile uint64_t limine_base_revision[] = LIMINE_BASE_REVISION(4);

__attribute__((used, section(".limine_requests")))
static volatile struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST_ID,
    .revision = 0
};

__attribute__((used, section(".limine_requests")))
static volatile struct limine_bootloader_info_request bootloader_info_request = {
    .id = LIMINE_BOOTLOADER_INFO_REQUEST_ID,
    .revision = 0
};

__attribute__((used, section(".limine_requests")))
static volatile struct limine_firmware_type_request firmware_type_request = {
    .id = LIMINE_FIRMWARE_TYPE_REQUEST_ID,
    .revision = 0
};

__attribute__((used, section(".limine_requests")))
volatile struct limine_memmap_request memmap_request = {
    .id = LIMINE_MEMMAP_REQUEST_ID,
    .revision = 0
};

__attribute__((used, section(".limine_requests")))
static volatile struct limine_module_request module_request = {
    .id = LIMINE_MODULE_REQUEST_ID,
    .revision = 0,
};

__attribute__((used, section(".limine_requests")))
volatile struct limine_hhdm_request hhdm_request = {
    .id = LIMINE_HHDM_REQUEST_ID,
    .revision = 0
};

__attribute__((used, section(".limine_requests")))
volatile struct limine_rsdp_request rsdp_request = {
    .id = LIMINE_RSDP_REQUEST_ID,
    .revision = 0
};

__attribute__((used, section(".limine_requests_start")))
static volatile uint64_t limine_requests_start_marker[] = LIMINE_REQUESTS_START_MARKER;

__attribute__((used, section(".limine_requests_end")))
static volatile uint64_t limine_requests_end_marker[] = LIMINE_REQUESTS_END_MARKER;

void hcf(void) {
    for (;;) {
        asm ("hlt");
    }
}

static void print_system_info(struct limine_framebuffer *fb) {
    fb_print_value("SonnaOS", "https://github.com/eteriaal/SonnaOS", COL_TITLE, 0x20B2AA);
    fb_print("\n", 0);

    fb_print(ESTELLA_VERSION " x86_64 EFI | limine protocol\n", COL_VERSION);
    fb_print("Using Spleen font 12x24 .psfu (psf2)\n", COL_INFO);

    fb_print("\n", 0);

    if (bootloader_info_request.response) {
        char bootloader_str[64];
        {
            size_t pos = 0;

            const char *name = bootloader_info_request.response->name;
            const char *version = bootloader_info_request.response->version;

            for (size_t i = 0; name[i] && pos + 1 < sizeof(bootloader_str); i++)
                bootloader_str[pos++] = name[i];

            if (pos + 1 < sizeof(bootloader_str))
                bootloader_str[pos++] = ' ';

            for (size_t i = 0; version[i] && pos + 1 < sizeof(bootloader_str); i++)
                bootloader_str[pos++] = version[i];

            bootloader_str[pos < sizeof(bootloader_str) ? pos : sizeof(bootloader_str) - 1] = '\0'; 
        }
        fb_print_value("Bootloader: ", bootloader_str, COL_LABEL, COL_VALUE);
        fb_print("\n", 0);

        serial_puts("Bootloader: ");
        serial_puts(bootloader_str);
        serial_puts("\n");
    }

    if (firmware_type_request.response) {
        const char *fw = "Unknown";
        switch (firmware_type_request.response->firmware_type) {
            case LIMINE_FIRMWARE_TYPE_X86BIOS: fw = "BIOS"; break;
            case LIMINE_FIRMWARE_TYPE_EFI32: fw = "EFI32"; break;
            case LIMINE_FIRMWARE_TYPE_EFI64: fw = "EFI64"; break;
            case LIMINE_FIRMWARE_TYPE_SBI: fw = "SBI"; break;
        }
        fb_print_value("Firmware:   ", fw, COL_LABEL, COL_VALUE);
        fb_print("\n", 0);

        serial_puts("Firmware: ");
        serial_puts(fw);
        serial_puts("\n");
    }

    char fb_addr[20] = {0};
    u64_to_hex((uint64_t)fb->address, fb_addr);
    fb_print_value("Framebuffer:", fb_addr, COL_LABEL, COL_ADDRESS);
    fb_print("\n\n", 0);
}

static void print_memory_info(void) {
    size_t total  = pmm_get_total_frames();
    size_t usable = pmm_get_usable_frames();
    size_t free   = pmm_get_free_frames();
    size_t used   = pmm_get_used_frames();

    char buf[80];

    fb_print("Memory Overview\n", COL_SECTION_TITLE);

    u64_to_dec(total * PAGE_SIZE / 1024 / 1024, buf);
    strcat(buf, " MiB (");
    u64_to_dec(total, buf + strlen(buf));
    strcat(buf, " pages)");
    fb_print_value("> Total   ", buf, COL_LABEL, COL_VALUE);
    fb_print("\n", 0);

    u64_to_dec(usable * PAGE_SIZE / 1024 / 1024, buf);
    strcat(buf, " MiB (");
    u64_to_dec(usable, buf + strlen(buf));
    strcat(buf, " pages)");
    fb_print_value("> Usable  ", buf, COL_LABEL, COL_VALUE);
    fb_print("\n", 0);

    u64_to_dec(used * PAGE_SIZE / 1024 / 1024, buf);
    strcat(buf, " MiB (");
    u64_to_dec(used, buf + strlen(buf));
    strcat(buf, " pages)");
    fb_print_value("> Used    ", buf, COL_LABEL, COL_USED);
    fb_print("\n", 0);

    u64_to_dec(free * PAGE_SIZE / 1024 / 1024, buf);
    strcat(buf, " MiB (");
    u64_to_dec(free, buf + strlen(buf));
    strcat(buf, " pages)");
    fb_print_value("> Free    ", buf, COL_LABEL, COL_FREE);
    fb_print("\n\n", 0);

    serial_puts("Memory: total ");
    u64_to_dec(total * PAGE_SIZE / 1024 / 1024, buf);
    serial_puts(buf);
    serial_puts(" MiB (");
    u64_to_dec(total, buf);
    serial_puts(buf);
    serial_puts(" pages), usable ");
    u64_to_dec(usable * PAGE_SIZE / 1024 / 1024, buf);
    serial_puts(buf);
    serial_puts(" MiB (");
    u64_to_dec(usable, buf);
    serial_puts(buf);
    serial_puts(" pages), free ");
    u64_to_dec(free * PAGE_SIZE / 1024 / 1024, buf);
    serial_puts(buf);
    serial_puts(" MiB (");
    u64_to_dec(free, buf);
    serial_puts(buf);
    serial_puts(" pages)\n");
}

void run_pmm_tests(void) {
    void *p1 = pmm_alloc();
    if (!p1) goto fail;
    pmm_free(p1);

    void *p2 = pmm_alloc_frames(4);
    if (!p2) goto fail;
    pmm_free_frames(p2, 4);

    void *p3 = pmm_alloc_frames_aligned(4, PAGE_SIZE * 4);
    if (!p3 || ((uintptr_t)p3 % (PAGE_SIZE * 4) != 0)) goto fail;
    pmm_free_frames(p3, 4);

    void *p4 = pmm_alloc_zeroed();
    if (!p4) goto fail;
    if (*(uint64_t*)phys_to_virt((uint64_t)p4) != 0) goto fail;
    pmm_free(p4);

    fb_print("PMM tests: OK\n", COL_SUCCESS_INIT);
    serial_puts("PMM tests OK\n");
    return;

fail:
    fb_print("PMM tests: FAILED\n", COL_FAIL);
    serial_puts("PMM tests FAILED\n");
}

void run_vmm_tests(void) {
    uint64_t vaddr = 0xFFFF900000000000ULL;
    void *phys = pmm_alloc();
    if (!phys) goto fail;

    if (!vmm_map(vaddr, (uint64_t)phys, PTE_KERNEL_RW)) goto fail_free;

    uint64_t *ptr = (uint64_t *)vaddr;
    *ptr = 0xDEADBEEFCAFEBABELL;
    if (*ptr != 0xDEADBEEFCAFEBABELL) goto fail_unmap;

    vmm_unmap(vaddr);
    pmm_free(phys);

    fb_print("VMM tests: OK\n", COL_SUCCESS_INIT);
    serial_puts("VMM tests OK\n");
    return;

fail_unmap:
    vmm_unmap(vaddr);
fail_free:
    pmm_free(phys);
fail:
    fb_print("VMM tests: FAILED\n", COL_FAIL);
    serial_puts("VMM tests FAILED\n");
}

static void trigger_panic(void) {
    fb_print("Testing #TS (invalid TSS) - 13 vector...\n", 0xFFFF00);
    serial_puts("Testing #TS (invalid TSS) - 13 vector...\n");
    asm volatile(
        "mov $0x28, %%ax\n\t"
        "ltr %%ax\n\t"
        "mov $0xFFFF, %%ax\n\t"
        "ltr %%ax"
        : : : "ax"
    );
}

void EstellaEntry(void) {
    // asm volatile("sti");
    // https://codeberg.org/Limine/limine-protocol/src/branch/trunk/PROTOCOL.md#x86-64-1
    // IF flag is cleared on entry

    serial_init();
    serial_puts("EstellaEntry\n");

    if (!LIMINE_BASE_REVISION_SUPPORTED(limine_base_revision)) hcf();
    if (!framebuffer_request.response || framebuffer_request.response->framebuffer_count == 0) hcf();
    if (!hhdm_request.response || !memmap_request.response || !module_request.response || !rsdp_request.response) hcf();

    // load font, init fbtext
    struct limine_file *font_module;
    struct psf2_header *psf2 = load_psf2_font(module_request, &font_module);
    if (!psf2) {
        serial_puts("!psf2\n");
        hcf();
    }
    font_t font = {
        .is_psf2 = true,
        .hdr.psf2 = psf2,
        .glyphs = (const uint8_t *)font_module->address + psf2->headersize,
        .width = psf2->width,
        .height = psf2->height,
        .line_height = psf2->height + 1,
        .glyph_count = psf2->length
    };
    struct limine_framebuffer *fb = framebuffer_request.response->framebuffers[0];
    fbtext_init(fb, &font);

    // init everything
    gdt_init(); fb_print("GDT + TSS initialized\n", COL_SUCCESS_INIT); serial_puts("GDT + TSS initialized\n");
    idt_init(); fb_print("IDT + ISR initialized\n", COL_SUCCESS_INIT); serial_puts("IDT + ISR initialized\n");
    pmm_init(); fb_print("PMM initialized\n", COL_SUCCESS_INIT); serial_puts("PMM initialized\n");
    vmm_init(); fb_print("VMM initialized\n", COL_SUCCESS_INIT); serial_puts("VMM initialized\n");
    apic_init(); fb_print("LAPIC + IOAPIC initialized\n", COL_SUCCESS_INIT); serial_puts("LAPIC + IOAPIC initialized\n");
    keyboard_init(); fb_print("Keyboard initialized\n", COL_SUCCESS_INIT); serial_puts("Keyboard initialized\n");

    run_pmm_tests();
    run_vmm_tests();

    fb_print("\n", 0);
    print_system_info(fb);
    print_memory_info();

    // Enabling interrupts
    asm volatile("sti");

    fb_print("Press any key to trigger kernel panic from #TS\n", COL_INFO);
    serial_puts("Press any key to trigger kernel panic from #TS\n");
    while (1) {
        if (keyboard_has_data()) {
            char ch = keyboard_get_char();
            if (ch != 0) {
                fb_print("Pressed: '", COL_INFO);
                char tmp[2] = {ch, '\0'};
                fb_print(tmp, COL_USED);
                fb_print("'\n", COL_INFO);

                serial_puts("Pressed: '");
                serial_putc(ch);
                serial_puts("'\n");

                fb_print("\n", 0);
                serial_puts("\n");

                trigger_panic();
            }
        }
    }
    hcf();
}