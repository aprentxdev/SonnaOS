OUTPUT_FORMAT(elf64-x86-64)
OUTPUT_ARCH(i386:x86-64)

ENTRY(user_main)

SECTIONS
{
    . = 0x400000;
    .text : ALIGN(0x1000)
    {
        *(.text .text.*)
        KEEP(*(.text.startup))
        KEEP(*(.text.user_main))
    }

    .rodata : ALIGN(0x1000)
    {
        *(.rodata .rodata.*)
        *(.rodata*)
    }

    .data : ALIGN(0x1000)
    {
        *(.data .data.*)
    }

    .bss : ALIGN(0x1000)
    {
        *(.bss .bss.*)
        *(COMMON)
    }

    /DISCARD/ :
    {
        *(.eh_frame)
        *(.eh_frame_hdr)
        *(.note.*)
        *(.comment)
        *(.rela*)
        *(.rel*)
    }
}