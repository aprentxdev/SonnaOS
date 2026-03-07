#include <fs/cpio/cpio.h>
#include <limine.h>
#include <klib/string.h>
#include <stdint.h>
#include <drivers/serial.h>

static uint32_t cpio_parse_hex(const unsigned char *field, int len) {
    uint32_t result = 0;
    for (int i = 0; i < len; i++) {
        unsigned char c = field[i];
        uint32_t digit;
        if (c >= '0' && c <= '9')      digit = c - '0';
        else if (c >= 'a' && c <= 'f') digit = c - 'a' + 10;
        else if (c >= 'A' && c <= 'F') digit = c - 'A' + 10;
        else break;
        result = (result << 4) | digit;
    }
    return result;
}

void* cpio_lookup(struct limine_file *module, const char *path, size_t *out_size) {
    if(!module) return NULL;
    uint8_t *ptr = (uint8_t *)module->address;
    while(1) {
        struct cpio_header* hdr = (struct cpio_header*)ptr;
        if(strncmp((const char*)hdr->c_magic, "070701", 6) != 0) {
            serial_puts("[cpio_lookup] bad magic\n");
            return NULL;
        }

        uint32_t namesize = cpio_parse_hex(hdr->c_namesize, 8);
        uint32_t filesize = cpio_parse_hex(hdr->c_filesize, 8);
        const char *name = (const char *)(ptr + sizeof(cpio_header));
        if (strcmp(name, "TRAILER!!!") == 0) {
            return NULL;
        }

        void *data = ptr + ((sizeof(cpio_header) + namesize + 3) & ~3);

        if(strcmp(name, path) == 0) {
            if(out_size) *out_size = filesize;
            return data;
        }

        ptr += ((sizeof(cpio_header) + namesize + 3) & ~3) + ((filesize + 3) & ~3);
    }

    return NULL;
}