#include <arch/x86_64/idt.h>
#include <drivers/fbtext.h>
#include <klib/string.h>
#include <klib/memory.h>

#define IDT_ENTRIES 256
#define IDT_INTERRUPT 0x8E

static struct idt_entry idt[IDT_ENTRIES];
static struct idtr idtr;

extern void isr0(void), isr6(void), isr8(void), isr13(void), isr14(void);

void exception_handler(uint64_t vector, uint64_t error, uint64_t rip) {
    fb_print("KERNEL PANIC!\n\n", 0xFF5555);

    char buf[32];

    memset(buf, 0, sizeof(buf));
    u64_to_dec(vector, buf);
    fb_print_value("Vector:", buf, 0xCCCCCC, 0xFF5555);
    fb_print("\n", 0);

    memset(buf, 0, sizeof(buf));
    u64_to_hex(error, buf);
    fb_print_value("Error code:", buf, 0xCCCCCC, 0xFF5555);
    fb_print("\n", 0);

    memset(buf, 0, sizeof(buf));
    u64_to_hex(rip, buf);
    fb_print_value("RIP:", buf, 0xCCCCCC, 0xFF5555);
    fb_print("\n", 0);

    fb_print("System halted.", 0xFF5555);

    while (1) asm volatile("hlt");
}

static void idt_set_gate(uint8_t n, void *handler, uint8_t ist) {
    uint64_t addr = (uint64_t)handler;
    idt[n].offset_low  = addr & 0xFFFF;
    idt[n].selector    = 0x08;
    idt[n].ist         = ist;
    idt[n].type_attr   = IDT_INTERRUPT;
    idt[n].offset_mid  = (addr >> 16) & 0xFFFF;
    idt[n].offset_high = addr >> 32;
    idt[n].zero        = 0;
}

void idt_init(void) {
    for (int i = 0; i < IDT_ENTRIES; i++) {
        idt_set_gate(i, isr0, 0);
    }

    idt_set_gate(0,  isr0,  0); // divide error
    idt_set_gate(6,  isr6,  0); // invalid opcode
    idt_set_gate(8,  isr8,  1); // double fault
    idt_set_gate(13, isr13, 0); // general protection fault
    idt_set_gate(14, isr14, 0); // page fault

    idtr.limit = sizeof(idt) - 1;
    idtr.base  = (uint64_t)&idt;

    asm volatile ("lidt %0" : : "m"(idtr));
}