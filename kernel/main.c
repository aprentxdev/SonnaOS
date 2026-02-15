#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include <limine.h>

#include <klib/memory.h>
#include <klib/string.h>
#include <arch/x86_64/gdt.h>
#include <arch/x86_64/idt.h>
#include <arch/x86_64/acpi.h>
#include <drivers/font.h>
#include <drivers/fbtext.h>
#include <drivers/serial.h>
#include <mm/pmm.h>
#include <mm/vmm.h>
#include <colors.h>

#define ESTELLA_VERSION "v0.Estella.5.0"

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
static volatile struct limine_rsdp_request rsdp_request = {
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
    fb_print("Running PMM tests...\n", COL_INFO);
    char buf[32];

    void *phys1 = pmm_alloc();
    if (phys1) {
        fb_print("PMM: alloc OK\n", COL_SUCCESS_INIT);
        serial_puts("PMM: alloc OK\n");
        pmm_free(phys1);
        fb_print("PMM: free OK\n", COL_SUCCESS_INIT);
        serial_puts("PMM: free OK\n");
    } else {
        fb_print("PMM: alloc FAILED (OOM?)\n", COL_FAIL);
        serial_puts("PMM: alloc FAILED (OOM?)\n");
        return;
    }

    size_t test_count = 4;
    void *phys_contig = pmm_alloc_frames(test_count);
    if (phys_contig) {
        fb_print("PMM: contiguous OK\n", COL_SUCCESS_INIT);
        serial_puts("PMM contiguous OK\n");
        pmm_free_frames(phys_contig, test_count);
    } else {
        fb_print("PMM: contiguous FAILED (fragmentation?)\n", COL_FAIL);
        serial_puts("PMM: contiguous FAILED (fragmentation?)\n");
        return;
    }

    size_t align_test = PAGE_SIZE * 4;
    void *phys_aligned = pmm_alloc_frames_aligned(test_count, align_test);
    if (phys_aligned && ((uint64_t)phys_aligned % align_test == 0)) {
        fb_print("PMM: aligned OK\n", COL_SUCCESS_INIT);
        serial_puts("PMM aligned OK\n");
        pmm_free_frames(phys_aligned, test_count);
    } else {
        fb_print("PMM: aligned FAILED (misaligned or OOM)\n", COL_FAIL);
        serial_puts("PMM: aligned FAILED (misaligned or OOM)\n");
        if (phys_aligned) pmm_free_frames(phys_aligned, test_count);
        return;
    }

    void *phys_zero = pmm_alloc_zeroed();
    if (phys_zero) {
        uint64_t *ptr = (uint64_t *)phys_to_virt((uint64_t)phys_zero);
        if (*ptr == 0) {
            fb_print("PMM: zeroed OK\n", COL_SUCCESS_INIT);
            serial_puts("PMM zeroed OK\n");
        } else {
            fb_print("PMM: zeroed FAILED (not zeroed)\n", COL_FAIL);
            serial_puts("PMM zeroed: first word = "); u64_to_hex(*ptr, buf); serial_puts(buf); serial_puts("\n");
        }
        pmm_free(phys_zero);
    } else {
        fb_print("PMM: zeroed FAILED (OOM)\n", COL_FAIL);
        serial_puts("PMM: zeroed FAILED (OOM)\n");
        return;
    }

    fb_print("PMM tests: PASSED\n\n", COL_SUCCESS_INIT);
}

void run_vmm_tests(void) {
    fb_print("Running VMM tests...\n", COL_INFO);
    serial_puts("Running VMM tests...\n");

    char buf[32];

    uint64_t virt_small = 0xFFFF900000000000ULL;
    void *phys_small = pmm_alloc();
    if (!phys_small || !vmm_map(virt_small, (uint64_t)phys_small, PTE_KERNEL_RW)) {
        fb_print("VMM: basic map FAILED (OOM or invalid addr)\n", COL_FAIL);
        serial_puts("VMM basic map failed (OOM or invalid addr)\n");
        if (phys_small) pmm_free(phys_small);
        return;
    }
    fb_print("VMM: basic map OK\n", COL_SUCCESS_INIT);
    serial_puts("VMM: basic map OK\n");

    uint64_t *ptr_small = (uint64_t *)virt_small;
    *ptr_small = 0xDEADBEEF12345678ULL;
    if (*ptr_small == 0xDEADBEEF12345678ULL) {
        fb_print("VMM: basic R/W OK\n", COL_SUCCESS_INIT);
        serial_puts("VMM: basic R/W OK\n");
    } else {
        fb_print("VMM: basic R/W FAILED (mismatch)\n", COL_FAIL);
        serial_puts("VMM: basic R/W FAILED (mismatch)\n");
    }

    uint64_t read_phys = vmm_get_physical(virt_small);
    if (read_phys == (uint64_t)phys_small) {
        fb_print("VMM: get_physical OK\n", COL_SUCCESS_INIT);
        serial_puts("VMM: get_physical OK\n");
    } else {
        fb_print("VMM: get_physical FAILED (wrong phys)\n", COL_FAIL);
        serial_puts("VMM get_phys: expected "); u64_to_hex((uint64_t)phys_small, buf); serial_puts(buf);
        serial_puts(", got "); u64_to_hex(read_phys, buf); serial_puts(buf); serial_puts("\n");
    }

    vmm_unmap(virt_small);
    pmm_free(phys_small);
    fb_print("VMM: basic unmap OK\n", COL_SUCCESS_INIT);
    serial_puts("VMM: basic unmap OK\n");

    size_t huge_frames = HUGE_2MB / PAGE_SIZE;
    void *phys_huge = pmm_alloc_frames_aligned_zeroed(huge_frames, HUGE_2MB);
    if (!phys_huge) {
        fb_print("VMM: huge alloc FAILED (no aligned 2MiB)\n", COL_FAIL);
        serial_puts("VMM: huge alloc FAILED (no aligned 2MiB)\n");
        return;
    }
    uint64_t phys_huge_addr = (uint64_t)phys_huge;
    if (phys_huge_addr % HUGE_2MB != 0) {
        fb_print("VMM: huge align FAILED (not 2MiB aligned)\n", COL_FAIL);
        serial_puts("VMM: huge align FAILED (not 2MiB aligned)\n");
        pmm_free_frames(phys_huge, huge_frames);
        return;
    }
    fb_print("VMM: huge alloc OK\n", COL_SUCCESS_INIT);
    serial_puts("VMM: huge alloc OK\n");

    uint64_t virt_huge = 0xFFFFA00000000000ULL;
    if (vmm_map_huge_2mb(virt_huge, phys_huge_addr, PTE_KERNEL_RW)) {
        fb_print("VMM: huge map OK\n", COL_SUCCESS_INIT);
        serial_puts("VMM: huge map OK\n");
        uint64_t *ptr = (uint64_t *)virt_huge;
        *ptr = 0xCAFEBABE87654321ULL;
        if (*ptr == 0xCAFEBABE87654321ULL) {
            fb_print("VMM: huge R/W OK\n", COL_SUCCESS_INIT);
            serial_puts("VMM: huge R/W OK\n");
        } else {
            fb_print("VMM: huge R/W FAILED (mismatch)\n", COL_FAIL);
            serial_puts("VMM: huge R/W FAILED (mismatch)\n");
        }

        uint64_t read_phys = vmm_get_physical(virt_huge);
        if ((read_phys & ~(HUGE_2MB-1)) == phys_huge_addr) {
            fb_print("VMM: huge get_physical OK\n", COL_SUCCESS_INIT);
            serial_puts("VMM: huge get_physical OK\n");
        } else {
            fb_print("VMM: huge get_physical FAILED (wrong base)\n", COL_FAIL);
            serial_puts("VMM: huge get_physical FAILED (wrong base)\n");
        }

        vmm_unmap_huge_2mb(virt_huge);
        pmm_free_frames(phys_huge, huge_frames);
        fb_print("VMM: huge unmap OK\n", COL_SUCCESS_INIT);
        serial_puts("VMM: huge unmap OK\n");
    } else {
        fb_print("VMM: huge map FAILED\n", COL_FAIL);
        serial_puts("VMM huge map FAILED\n");
        pmm_free_frames(phys_huge, huge_frames);
        return;
    }

    void *phys_double = pmm_alloc();
    if (phys_double && vmm_map(virt_small, (uint64_t)phys_double, PTE_KERNEL_RW)) {
        if (!vmm_map(virt_small, (uint64_t)phys_double, PTE_KERNEL_RW)) {
            fb_print("VMM: double map rejected OK\n", COL_SUCCESS_INIT);
            serial_puts("VMM double map rejected OK\n");
        } else {
            fb_print("VMM: double map ALLOWED (FAIL!)\n", COL_FAIL);
            serial_puts("VMM: double map ALLOWED (FAIL!)\n");
        }
        vmm_unmap(virt_small);
        pmm_free(phys_double);
    } else {
        fb_print("VMM: double map setup FAILED\n", COL_FAIL);
        serial_puts("VMM double map setup FAILED\n");
        if (phys_double) pmm_free(phys_double);
        return;
    }

    fb_print("VMM tests: PASSED\n\n", COL_SUCCESS_INIT);
    serial_puts("VMM tests: PASSED\n\n");
}

__attribute__((unused)) 
static void run_exception_test(void) {
    // fb_print("Testing #UD (invalid opcode (ud2)) - 6 vector...\n", COL_TEST_HDR);
    // serial_puts("Testing #UD (invalid opcode (ud2)) - 6 vector...\n");
    // asm volatile("ud2");

    // fb_print("Testing #DE (divide by zero) - 0 vector...\n", COL_TEST_HDR);
    // serial_puts("Testing #DE (divide by zero) - 0 vector...\n");
    // asm volatile("mov $0, %%eax; idiv %%eax" : : : "eax");

    // fb_print("Testing #GP (invalid selector) - 13 vector...\n", 0xFFFF00);
    // serial_puts("Testing #GP (invalid selector) - 13 vector...\n");
    // asm volatile("mov $0x28, %%ax; mov %%ax, %%fs; mov %%fs:0, %%rax" : : : "rax", "ax");

    // fb_print("Testing #PF (dereference null) - 14 vector...\n", 0xFFFF00);
    // serial_puts("Testing #PF (dereference null) - 14 vector...\n");
    // volatile uint64_t *null_ptr = (uint64_t *)0x0;
    // uint64_t dummy = *null_ptr;

    // fb_print("Testing #BP (int3) - 3 vector...\n", 0xFFFF00);
    // serial_puts("Testing #BP (int3) - 3 vector...\n");
    // asm volatile("int3");

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

static void test_acpi(struct limine_rsdp_response *rsdp_resp) {
    char buf[32];
    void *rsdp_ptr = rsdp_resp->address;

    struct xsdt* xsdt = acpi_get_xsdt(rsdp_ptr);
    if (!xsdt) {
        serial_puts("XSDT not found!\n");
        return;
    }

    serial_puts("XSDT found: ");
    u64_to_hex((uint64_t)xsdt, buf);
    serial_puts(buf);
    serial_puts(", length = ");
    u64_to_dec(xsdt->header.length, buf);
    serial_puts(buf);
    serial_puts("\n");

    struct madt* madt = acpi_get_madt(rsdp_ptr);
    if (!madt) {
        serial_puts("MADT not found!\n");
        return;
    }

    serial_puts("MADT found: ");
    u64_to_hex((uint64_t)madt, buf);
    serial_puts(buf);
    serial_puts("\nLAPIC address = ");
    u64_to_hex((uint64_t)madt->lapic_address, buf);
    serial_puts(buf);
    serial_puts("\nflags = ");
    u64_to_hex((uint64_t)madt->flags, buf);
    serial_puts(buf);
    serial_puts("\n");
}

void EstellaEntry(void) {
    serial_init();
    serial_puts("EstellaEntry\n");
    if (!LIMINE_BASE_REVISION_SUPPORTED(limine_base_revision)) hcf();
    if (!framebuffer_request.response || framebuffer_request.response->framebuffer_count == 0) hcf();
    if (!hhdm_request.response || !memmap_request.response || !module_request.response || !rsdp_request.response) hcf();

    
    struct limine_framebuffer *fb = framebuffer_request.response->framebuffers[0];

    struct limine_file *font_module;
    struct psf2_header *psf2 = load_psf2_font(module_request, &font_module);
    if (!psf2) {serial_puts("!psf2\n");hcf();}

    font_t font = {
        .is_psf2 = true,
        .hdr.psf2 = psf2,
        .glyphs = (const uint8_t *)font_module->address + psf2->headersize,
        .width = psf2->width,
        .height = psf2->height,
        .line_height = psf2->height + 1,
        .glyph_count = psf2->length
    };
    fbtext_init(fb, &font);

    gdt_init(); fb_print("GDT + TSS initialized\n", COL_SUCCESS_INIT); serial_puts("GDT + TSS initialized\n");
    idt_init(); fb_print("IDT + ISR initialized\n", COL_SUCCESS_INIT); serial_puts("IDT + ISR initialized\n");
    pmm_init(); fb_print("PMM initialized\n", COL_SUCCESS_INIT); serial_puts("PMM initialized\n");
    vmm_init(); fb_print("VMM initialized\n", COL_SUCCESS_INIT); serial_puts("VMM initialized\n");

    struct limine_rsdp_response *rsdp_resp = rsdp_request.response;
    test_acpi(rsdp_resp);
    
    fb_print("\n", 0);

    print_system_info(fb);
    print_memory_info();

    run_pmm_tests();
    run_vmm_tests();

    // run_exception_test();

    hcf();
}