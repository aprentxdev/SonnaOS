#ifndef ESTELLA_MM_PMM_H
#define ESTELLA_MM_PMM_H

#include <stdint.h>
#include <stddef.h>

#include <limine.h>

#define PAGE_SIZE 4096ULL

void pmm_init();

void* pmm_alloc(void);
void *pmm_alloc_zeroed(void);
void *pmm_alloc_aligned(size_t bytes, size_t alignment);
void *pmm_alloc_aligned_zeroed(size_t bytes, size_t alignment);

void* pmm_alloc_frames(size_t count);
void *pmm_alloc_frames_zeroed(size_t count);
void *pmm_alloc_frames_aligned(size_t count, size_t alignment);
void *pmm_alloc_frames_aligned_zeroed(size_t count, size_t alignment);

void pmm_free(void* phys_addr);
void pmm_free_frames(void* phys_addr, size_t count);

size_t pmm_get_total_frames(void);
size_t pmm_get_free_frames(void);
size_t pmm_get_usable_frames(void);
size_t pmm_get_used_frames(void);

#endif