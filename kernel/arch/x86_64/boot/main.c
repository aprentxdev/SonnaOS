#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include <limine.h>

#include <klib/memory.h>
#include <klib/string.h>
#include <arch/x86_64/cpu/gdt.h>
#include <arch/x86_64/interrupts/idt.h>
#include <arch/x86_64/acpi/acpi.h>
#include <arch/x86_64/interrupts/apic.h>
#include <drivers/font.h>
#include <drivers/fbtext.h>
#include <drivers/serial.h>
#include <drivers/keyboard.h>
#include <mm/pmm.h>
#include <arch/x86_64/mm/vmm.h>
#include <colors.h>
#include <shell_kspace/kernelshell.h>
#include <arch/x86_64/time/time.h>
#include <arch/x86_64/syscalls/syscalls.h>
#include <arch/x86_64/usermode/usermode.h>
#include <arch/x86_64/usermode/scheduler.h>

#define ESTELLA_VERSION "Estella v0.9.0-dev"

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

__attribute__((used, section(".limine_requests")))
volatile struct limine_date_at_boot_request date_at_boot_request = {
    .id = LIMINE_DATE_AT_BOOT_REQUEST_ID,
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

static void print_system_info(struct limine_framebuffer *fb, struct limine_file *logo) {
    fb_print("SonnaOS " ESTELLA_VERSION " (x86_64 UEFI)\n", COL_VERSION);
    fb_print("Development on: https://github.com/aprentxdev/SonnaOS\n", COL_VERSION);

    if (firmware_type_request.response) {
        const char *fw = "Unknown";
        switch (firmware_type_request.response->firmware_type) {
            case LIMINE_FIRMWARE_TYPE_X86BIOS: fw = "BIOS"; break;
            case LIMINE_FIRMWARE_TYPE_EFI32: fw = "EFI32"; break;
            case LIMINE_FIRMWARE_TYPE_EFI64: fw = "EFI64"; break;
            case LIMINE_FIRMWARE_TYPE_SBI: fw = "SBI"; break;
        }
        fb_print("Firmware: ", COL_LABEL);
        fb_print(fw, COL_VALUE);
        fb_print("\n", 0);

        serial_puts("Firmware: ");
        serial_puts(fw);
        serial_puts("\n");
    }
    
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

        fb_print("Bootloader: ", COL_LABEL);
        fb_print(bootloader_str, COL_VALUE);
        fb_print("\n", 0);

        serial_puts("Bootloader: ");
        serial_puts(bootloader_str);
        serial_puts("\n");
    }

    if (!logo || !logo->address) return;

    #define LOGO_W 447
    #define LOGO_H 126

    uint32_t logo_x = fb->width - LOGO_W - 750;
    uint32_t logo_y = 7;

    uint32_t *pixels = (uint32_t *)logo->address;
    uint32_t *fb_ptr = (uint32_t *)fb->address;
    uint32_t  stride = fb->pitch / sizeof(uint32_t);

    for (uint32_t y = 0; y < LOGO_H; y++) {
        for (uint32_t x = 0; x < LOGO_W; x++) {
            uint32_t pixel = pixels[y * LOGO_W + x];

            uint8_t a = (pixel >> 24) & 0xFF;
            if (a == 0) continue;
            if (a == 0xFF) {
                fb_ptr[(logo_y + y) * stride + (logo_x + x)] = pixel & 0x00FFFFFF;
            } else {
                uint32_t bg = fb_ptr[(logo_y + y) * stride + (logo_x + x)];
                uint8_t r = ((pixel >> 16) & 0xFF) * a / 255 + ((bg >> 16) & 0xFF) * (255 - a) / 255;
                uint8_t g = ((pixel >>  8) & 0xFF) * a / 255 + ((bg >>  8) & 0xFF) * (255 - a) / 255;
                uint8_t b = ((pixel >>  0) & 0xFF) * a / 255 + ((bg >>  0) & 0xFF) * (255 - a) / 255;
                fb_ptr[(logo_y + y) * stride + (logo_x + x)] = ((uint32_t)r << 16) | ((uint32_t)g << 8) | b;
            }
        }
    }

}

static void print_memory_info(void) {
    size_t total = pmm_get_total_frames();
    size_t usable = pmm_get_usable_frames();
    size_t free = pmm_get_free_frames();
    size_t used = pmm_get_used_frames();

    fb_print("Memory Overview\n", COL_SECTION_TITLE);

    fb_print("> Total   ", COL_LABEL);
    fb_print_number(total * PAGE_SIZE / 1024 / 1024, COL_VALUE);
    fb_print(" MiB (", COL_VALUE);
    fb_print_number(total, COL_VALUE);
    fb_print(" pages)\n", COL_VALUE);

    fb_print("> Usable  ", COL_LABEL);
    fb_print_number(usable * PAGE_SIZE / 1024 / 1024, COL_VALUE);
    fb_print(" MiB (", COL_VALUE);
    fb_print_number(usable, COL_VALUE);
    fb_print(" pages)\n", COL_VALUE);

    fb_print("> Used    ", COL_LABEL);
    fb_print_number(used * PAGE_SIZE / 1024 / 1024, COL_USED);
    fb_print(" MiB (", COL_USED);
    fb_print_number(used, COL_USED);
    fb_print(" pages)\n", COL_USED);

    fb_print("> Free    ", COL_LABEL);
    fb_print_number(free * PAGE_SIZE / 1024 / 1024, COL_FREE);
    fb_print(" MiB (", COL_FREE);
    fb_print_number(free, COL_FREE);
    fb_print(" pages)\n\n", COL_FREE);

    char buf[80];
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

int memorymanagers_tests(void) {
    // PMM
    void *p1 = pmm_alloc();
    if (!p1) goto pmm_fail;
    pmm_free(p1);

    void *p2 = pmm_alloc_frames(4);
    if (!p2) goto pmm_fail;
    pmm_free_frames(p2, 4);

    void *p3 = pmm_alloc_frames_aligned(4, PAGE_SIZE * 4);
    if (!p3 || ((uintptr_t)p3 % (PAGE_SIZE * 4) != 0)) goto pmm_fail;
    pmm_free_frames(p3, 4);

    void *p4 = pmm_alloc_zeroed();
    if (!p4) goto pmm_fail;
    if (*(uint64_t*)phys_to_virt((uint64_t)p4) != 0) goto pmm_fail;
    pmm_free(p4);


    // VMM
    uint64_t vaddr = 0xFFFF900000000000ULL;
    void *phys = pmm_alloc();
    if (!phys) goto vmm_fail;

    if (!vmm_map(vaddr, (uint64_t)phys, PTE_KERNEL_RW)) {pmm_free(phys); goto vmm_fail;}

    uint64_t *ptr = (uint64_t *)vaddr;
    *ptr = 0xDEADBEEFCAFEBABELL;
    if (*ptr != 0xDEADBEEFCAFEBABELL) {vmm_unmap(vaddr); goto vmm_fail;}

    vmm_unmap(vaddr);
    pmm_free(phys);
    return 0;

    pmm_fail:
        serial_puts("PMM tests FAILED\n");
        return 1;
    
    vmm_fail:
        serial_puts("VMM tests FAILED\n");
        return 1;

    return 0;
} 

void EstellaEntry(void) {
    // asm volatile("cli");
    // https://codeberg.org/Limine/limine-protocol/src/branch/trunk/PROTOCOL.md#x86-64-1
    // IF flag is cleared on entry

    serial_init();
    serial_puts("EstellaEntry\n");
    serial_puts("SonnaOS " ESTELLA_VERSION " (x86_64 UEFI)\n");
    serial_puts("Development on: https://github.com/aprentxdev/SonnaOS\n");

    if (!LIMINE_BASE_REVISION_SUPPORTED(limine_base_revision)) hcf();
    if (!framebuffer_request.response || framebuffer_request.response->framebuffer_count == 0) hcf();
    if (!hhdm_request.response || !memmap_request.response || !module_request.response || !rsdp_request.response) hcf();

    // modules
    int count_modules = module_request.response->module_count;
    struct limine_file *fontmod = NULL;
    struct limine_file *task_a_elf = NULL;
    struct limine_file *task_b_elf = NULL;
    struct limine_file *task_c_elf = NULL;
    struct limine_file *logo = NULL;
    
    for(int i = 0; i < count_modules; i++) {
        if (strcmp("spleen-12x24", module_request.response->modules[i]->string) == 0) {
            fontmod = module_request.response->modules[i];
        }
        if (strcmp("task_a", module_request.response->modules[i]->string) == 0) {
            task_a_elf = module_request.response->modules[i];
        }
        if (strcmp("task_b", module_request.response->modules[i]->string) == 0) {
            task_b_elf = module_request.response->modules[i];
        }
        if (strcmp("task_c", module_request.response->modules[i]->string) == 0) {
            task_c_elf = module_request.response->modules[i];
        }
        if (strcmp("logo_raw", module_request.response->modules[i]->string) == 0) {
            logo = module_request.response->modules[i];
        } 
    }

    // load font, init fbtext
    struct psf2_header *psf2 = load_psf2_font(fontmod);
    if (!psf2) {
        serial_puts("failed to load font?\n");
        hcf();
    }
    font_t font = {
        .is_psf2 = true,
        .hdr.psf2 = psf2,
        .glyphs = (const uint8_t *)fontmod->address + psf2->headersize,
        .width = psf2->width,
        .height = psf2->height,
        .line_height = psf2->height + 1,
        .glyph_count = psf2->length
    };
    struct limine_framebuffer *fb = framebuffer_request.response->framebuffers[0];
    fbtext_init(fb, &font);

    print_system_info(fb, logo); fb_print("\n", 0);

    // init everything
    gdt_init(); fb_print("GDT with TSS initialized;", COL_SUCCESS_INIT);
    idt_init(); fb_print(" IDT initialized;", COL_SUCCESS_INIT);
    syscalls_init(); fb_print(" Syscalls initialized;", COL_SUCCESS_INIT);
    pmm_init(); fb_print(" PMM initialized;", COL_SUCCESS_INIT); 
    vmm_init(); fb_print(" VMM initialized;", COL_SUCCESS_INIT); 
    apic_init(); fb_print(" TSC & APIC initialized;", COL_SUCCESS_INIT);
    time_init(); fb_print(" RTC initialized;", COL_SUCCESS_INIT);
    keyboard_init(); fb_print(" PS/2 keyboard driver initialized\n", COL_SUCCESS_INIT);

    if(memorymanagers_tests() == 0) fb_print("VMM & PMM tests ok\n\n", COL_SUCCESS_INIT);
    print_memory_info();

    // Enabling interrupts
    asm volatile("sti");
    scheduler_init(); fb_print("Scheduler initialized\n", COL_SUCCESS_INIT);

    task_t *t1 = task_create_from_elf(task_a_elf->address); fb_print("TASK_A created! ", COL_TITLE);
    task_t *t2 = task_create_from_elf(task_b_elf->address); fb_print("TASK_B created! ", COL_TITLE);
    task_t *t3 = task_create_from_elf(task_c_elf->address); fb_print("TASK_C created!\n", COL_TITLE);
    scheduler_add_task(t1); fb_print("TASK_A in queue! ", COL_TITLE);
    scheduler_add_task(t2); fb_print("TASK_B in queue! ", COL_TITLE);
    scheduler_add_task(t3); fb_print("TASK_C in queue!\n", COL_TITLE);

    fb_print("task_enter TASK_A\n", COL_TITLE);
    task_enter(t1);

    // launch_shell(); // kernelshell.h
    // hcf();
}