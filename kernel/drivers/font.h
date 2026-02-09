#ifndef KERNEL_FONT_H
#define KERNEL_FONT_H

#include <stdint.h>
#include <stddef.h>
#include <limine.h>

#define PSF1_MAGIC0 0x36
#define PSF1_MAGIC1 0x04

struct psf1_header {
    uint8_t magic[2];
    uint8_t mode;
    uint8_t charsize;
};


#define PSF2_MAGIC0 0x72
#define PSF2_MAGIC1 0xb5
#define PSF2_MAGIC2 0x4a
#define PSF2_MAGIC3 0x86

struct psf2_header {
    uint32_t magic[4];
    uint32_t version;
    uint32_t headersize;
    uint32_t flags;
    uint32_t length;
    uint32_t charsize;
    uint32_t height;
    uint32_t width;
};

struct psf1_header* load_psf1_font(struct limine_file **out_glyphs);
struct psf2_header* load_psf2_font(void);

#endif