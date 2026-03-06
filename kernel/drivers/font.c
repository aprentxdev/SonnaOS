#include <drivers/font.h>

struct psf1_header* load_psf1_font(struct limine_file *module) {
    if (!module) return NULL;

    struct psf1_header *hdr = (struct psf1_header*)module->address;

    if (hdr->magic[0] == PSF1_MAGIC0 && hdr->magic[1] == PSF1_MAGIC1) {
        return hdr;
    }

    return NULL;
}

struct psf2_header* load_psf2_font(struct limine_file *module) {
    if (!module) return NULL;
    if (module->size < sizeof(struct psf2_header)) return NULL;

    struct psf2_header *hdr = (struct psf2_header*)module->address;

    if (hdr->magic == PSF2_MAGIC) {
        if (module->size >= hdr->headersize + hdr->length * hdr->charsize) {
            return hdr;
        }
    }

    return NULL;
}