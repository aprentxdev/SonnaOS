#ifndef KERNEL_FBTEXT_H
#define KERNEL_FBTEXT_H

#include <stdint.h>
#include <stddef.h>

#include <limine.h>
#include <drivers/font.h>

void fbtext_init(struct limine_framebuffer *fb, struct psf1_header *font, const uint8_t *glyphs);
void fb_print(const char *str, uint32_t color);
void fb_print_value(const char *label, const char *value, uint32_t label_color, uint32_t value_color);
void fb_set_y(size_t y);

#endif