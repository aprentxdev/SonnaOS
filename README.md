# SonnaOS
Writing an operating system to explore low-level architecture and hardware programming. (because I want to occupy my time)

### Hardware Requirements
- **Firmware**: UEFI
- **CPU**: 64-bit x86 processor **with x2APIC** support
- **ACPI 2.0+**

## Kernel
**Estella** - x86_64 EFI kernel using the Limine bootloader protocol.

## Current status
### Boot & CPU setup
- Boots on x86_64 UEFI via Limine
- GDT + TSS initialized
- IDT + ISRs installed

### ACPI & Interrupts
- ACPI: RSDP / XSDT / MADT / HPET parsed
- x2APIC enabled
- LAPIC + IOAPIC initialized
- LAPIC running in TSC-deadline mode when invariant TSC is present

### Time subsystem
- TSC frequency detection via CPUID (0x15/0x16) when available
- HPET-based TSC calibration fallback
- TSC-based stopwatch utility

### Memory
- Physical Memory Manager (PMM)
- Virtual Memory Manager (VMM)
- Self-tests for memory subsystems

### Devices
- PS/2 keyboard driver
    - Basic debug key triggers:
        - t — toggle stopwatch
        - q — trigger panic

### Console
- Serial debug output
- Framebuffer text console (Spleen 12×24 .psfu)


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