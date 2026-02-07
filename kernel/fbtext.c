#include "fbtext.h"
#include "font.h"

void fb_put_char(struct limine_framebuffer *fb, struct psf1_header *font, const uint8_t *glyphs_addr,
                 char c, size_t x, size_t y, uint32_t fg_color) {
    uint32_t *fbp = fb->address;
    size_t pitch = fb->pitch / 4;

    // Glyphs start after the PSF1 header
    const uint8_t *glyphs = glyphs_addr + sizeof(struct psf1_header);
    const uint8_t *glyph  = glyphs + (size_t)c * font->charsize;

    for (uint32_t row = 0; row < font->charsize; row++)
    {
        uint8_t bits = glyph[row];

        for (uint32_t col = 0; col < 8; col++)
        {
            if (bits & (0x80 >> col))
            {
                fbp[(y + row) * pitch + (x + col)] = fg_color;
            }
        }
    }
}

void fb_put_string(struct limine_framebuffer *fb, struct psf1_header *font, const uint8_t *glyphs_addr,
                   const char *str, size_t x, size_t y, uint32_t fg_color) {
    size_t currentx = x;
    size_t currenty = y;

    for (size_t i = 0; str[i]; i++)
    {
        if (str[i] == '\n')
        {
            currentx = x;
            currenty += font->charsize;
            continue;
        }

        fb_put_char(fb, font, glyphs_addr, str[i], currentx, currenty, fg_color);
        currentx += 8; // Fixed width of 8 pixels for PSF1 character
    }
}