#ifndef ESTELLA_FS_CPIO_CPIO_H
#define ESTELLA_FS_CPIO_CPIO_H

#include <stdint.h>
#include <stddef.h>

#include <limine.h>

typedef struct cpio_header {
    unsigned char c_magic[6];
    unsigned char c_ino[8];
    unsigned char c_mode[8];
    unsigned char c_uid[8];
    unsigned char c_gid[8];
    unsigned char c_nlink[8];
    unsigned char c_mtime[8];
    unsigned char c_filesize[8];
    unsigned char c_maj[8];
    unsigned char c_min[8];
    unsigned char c_rmaj[8];
    unsigned char c_rmin[8];
    unsigned char c_namesize[8];
    unsigned char c_chksum[8];
} __attribute__((packed)) cpio_header;

void* cpio_lookup(struct limine_file *module, const char *path, size_t *out_size);

#endif