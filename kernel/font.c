#include "font.h"
#include "limine.h"
#include <stdint.h>
#include <stddef.h>

struct psf1_header* load_psf1_font(struct limine_file **out_glyphs) {
    extern struct limine_module_request module_request;
    if (!module_request.response) return NULL;

    for(size_t i = 0; i < module_request.response->module_count; i++) {
        struct limine_file *module = module_request.response->modules[i];
        struct psf1_header *hdr = (struct psf1_header*)module->address;

        if (hdr->magic[0] == PSF1_MAGIC0 && hdr->magic[1] == PSF1_MAGIC1) {
            *out_glyphs = module;
            return hdr;
        }
    }

    return NULL;
}

struct psf2_header* load_psf2_font() {
    return NULL;
}