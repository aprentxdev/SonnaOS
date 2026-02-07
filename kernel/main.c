// main.c v0.Estella.1.0 
//
// Entry point for the Estella kernel using the Limine bootloader protocol.
// It initializes the framebuffer, loads a PSF1 font module, and displays some text.

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <libkernel/memory.h>
#include "limine.h"
#include "fbtext.h"

// Base revision 4 (latest)
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
static volatile struct limine_memmap_request memmap_request = {
    .id = LIMINE_MEMMAP_REQUEST_ID,
    .revision = 0
};

__attribute__((used, section(".limine_requests")))
static volatile struct limine_mp_request mp_request = {
    .id = LIMINE_MP_REQUEST_ID,
    .revision = 0,
    .flags = 0
};

__attribute__((used, section(".limine_requests")))
static volatile struct limine_smbios_request smbios_request = {
    .id = LIMINE_SMBIOS_REQUEST_ID,
    .revision = 0
};

__attribute__((used, section(".limine_requests")))
static volatile struct limine_efi_system_table_request efi_system_table_request = {
    .id = LIMINE_EFI_SYSTEM_TABLE_REQUEST_ID,
    .revision = 0
};

__attribute__((used, section(".limine_requests")))
static volatile struct limine_date_at_boot_request date_request = {
    .id = LIMINE_DATE_AT_BOOT_REQUEST_ID,
    .revision = 0
};

// Module request - request for font file (module_path: boot():/boot/spleen/spleen-8x16.psfu)
__attribute__((used, section(".limine_requests")))
volatile struct limine_module_request module_request = {
    .id = LIMINE_MODULE_REQUEST_ID,
    .revision = 0,
};

__attribute__((used, section(".limine_requests_start")))
static volatile uint64_t limine_requests_start_marker[] = LIMINE_REQUESTS_START_MARKER;

__attribute__((used, section(".limine_requests_end")))
static volatile uint64_t limine_requests_end_marker[] = LIMINE_REQUESTS_END_MARKER;

// Halt and catch fire function. Infinite loop.
static void hcf(void) {
    for (;;) {
        asm ("hlt");
    }
}

void kmain(void) {
    if (LIMINE_BASE_REVISION_SUPPORTED(limine_base_revision) == false) {
        hcf();
    }

    if (framebuffer_request.response == NULL
     || framebuffer_request.response->framebuffer_count < 1) {
        hcf();
    }

    // Ensure module (font) is loaded.
    if (!module_request.response || module_request.response->module_count < 1) {
        hcf();
    }

    struct limine_file *font_module;
    struct psf1_header *font = load_psf1_font(&font_module);
    if (!font) hcf();

    struct limine_framebuffer *fb = framebuffer_request.response->framebuffers[0];

    fb_put_string(fb, font, (uint8_t*)font_module->address, "v0.Estella.1.0 x86_64 EFI via lumine protocol", 10, 10, 0xFFFFFF);
    fb_put_string(fb, font, (uint8_t*)font_module->address, "PSF1 SPLEEN 8x16", 10, 27, 0xFFFFFF);
    fb_put_string(fb, font, (uint8_t*)font_module->address, "Hello world!", 10,  44, 0xFFFFFF);

    // all done
    hcf();
}