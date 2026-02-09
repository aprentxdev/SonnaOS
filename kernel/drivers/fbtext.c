#include <drivers/fbtext.h>
#include <klib/string.h>

#define CHAR_WIDTH 8
#define LINE_HEIGHT 20
#define LEFT_MARGIN 20

static struct limine_framebuffer *g_fb = NULL;
static struct psf1_header *g_font = NULL;
static const uint8_t *g_glyphs = NULL;
static size_t g_cursor_x = LEFT_MARGIN;
static size_t g_cursor_y = 15;

void fbtext_init(struct limine_framebuffer *fb, struct psf1_header *font, const uint8_t *glyphs)
{
    g_fb = fb;
    g_font = font;
    g_glyphs = glyphs;
    g_cursor_x = LEFT_MARGIN;
    g_cursor_y = 15;
}

void fb_set_y(size_t y)
{
    g_cursor_y = y;
    g_cursor_x = LEFT_MARGIN;
}

static void fb_advance_cursor(size_t chars)
{
    g_cursor_x += chars * CHAR_WIDTH;
}

static void fb_newline(void)
{
    g_cursor_x = LEFT_MARGIN;
    g_cursor_y += LINE_HEIGHT;
}

static void fb_put_char(char c, uint32_t color)
{
    if (!g_fb || !g_font || !g_glyphs) return;

    if (g_cursor_x + CHAR_WIDTH > g_fb->width)
        fb_newline();

    if (g_cursor_y + g_font->charsize > g_fb->height)
        return;

    uint32_t *fbp = (uint32_t *)g_fb->address;
    size_t stride = g_fb->pitch / sizeof(uint32_t);

    const uint8_t *glyph = g_glyphs + sizeof(struct psf1_header) + ((uint8_t)c) * g_font->charsize;

    for (uint32_t row = 0; row < g_font->charsize; row++)
    {
        uint8_t bits = glyph[row];
        for (uint32_t col = 0; col < 8; col++)
        {
            if (bits & (0x80 >> col))
            {
                size_t px = g_cursor_x + col;
                size_t py = g_cursor_y + row;
                if (px < g_fb->width && py < g_fb->height)
                    fbp[py * stride + px] = color;
            }
        }
    }

    fb_advance_cursor(1);
}

void fb_print(const char *str, uint32_t color)
{
    if (!str) return;

    while (*str)
    {
        if (*str == '\n')
        {
            fb_newline();
            str++;
            continue;
        }

        fb_put_char(*str, color);
        str++;
    }
}

void fb_print_value(const char *label, const char *value, uint32_t label_color, uint32_t value_color)
{
    if (!label) return;

    fb_print(label, label_color);

    size_t label_len = strlen(label);
    size_t desired_x = LEFT_MARGIN + (label_len + 1) * CHAR_WIDTH;

    if (g_cursor_x < desired_x)
        g_cursor_x = desired_x;

    if (value && *value)
        fb_print(value, value_color);
}