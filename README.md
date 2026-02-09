# SonnaOS
Writing an operating system because I want to occupy my time...  

## Kernel
**Estella** - x86_64 EFI kernel using the Limine bootloader protocol.

### Current status
- Boots via Limine
- GDT + TSS (kernel and user segments)
- IDT + basic exception handling (#DE, #UD, #DF, #GP, #PF)
- Simple bitmap PMM with alloc/free tests
- Framebuffer console (Spleen 8x16 PSF1 font)
- Debug output: boot info, memory map, PMM tests

Next:
- IRQ handling
- Extend IDT
- PSF2 font support
- Serial console for better debugging
- Paging + virtual memory + kernel heap  

---
![screenshot](sonnaos.png)
---

### Requirements
- clang + ld.lld
- QEMU + OVMF
- make, git, curl/wget

### build & run
```bash
make run
```