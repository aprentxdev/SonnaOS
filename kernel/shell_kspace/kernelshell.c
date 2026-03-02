#include <klib/string.h>
#include <drivers/fbtext.h>
#include <drivers/keyboard.h>
#include <colors.h>
#include <generic/time.h>
#include <generic/usermode.h>

static void execute_command(const char* cmd) {
    if (!cmd || cmd[0] == '\0') return;

    if (strcmp(cmd, "help") == 0) {
        fb_print("Available commands: help, clear, about, time, panic, userspace, halt\n", 0x00FF00);
    }
    else if (strcmp(cmd, "clear") == 0) { 
        fb_clear();
    }
    else if (strcmp(cmd, "about") == 0) {
        fb_print("Estella shell (Ring 0)\n", 0x00BFFF);
    }
    else if (strcmp(cmd, "time") == 0) {
        char* time = time_get_current();
        fb_print("time (UTC): ", 0xFFAA00);
        fb_print(time, 0xFFFFFF);
        fb_print("\n", 0);
    }
    else if (strcmp(cmd, "panic") == 0) {
        fb_print("Triggering kernel panic...\n", 0xFF0000);
        asm volatile("ud2");
    }
    else if (strcmp(cmd, "halt") == 0) {
        fb_print("Halting system...\n", 0x00FFFF);
        asm volatile("cli; hlt");
        while (1) asm("hlt");
    }
    else if (strcmp(cmd, "userspace") == 0) {
        fb_print("Running elf...\n", COL_INFO);
        extern struct limine_file *user_module;
        enter_usermode(user_module->address, user_module->size);
    }
    else {
        fb_print("Unknown command: ", 0xAA0000);
        fb_print(cmd, 0xAAAAAA);
        fb_print("\n", 0);
    }
}

void launch_shell(void) {
    char cmd_buffer[256];
    unsigned int cmd_pos = 0;

    fb_print("Welcome to Estella kernel shell \n", COL_SUCCESS_INIT);
    fb_print("Type 'help' for commands.\n\n", COL_SUCCESS_INIT);
    
    fb_print("sh> ", 0x7FFFD4);

    while (1) {
        if (keyboard_has_data()) {
            char c = keyboard_get_char();
            if (c == '\n') {
                cmd_buffer[cmd_pos] = '\0';
                fb_put_char('\n', 0xAAAAAA);
                execute_command(cmd_buffer);
                cmd_pos = 0;
                fb_print("sh> ", 0x7FFFD4);
            }
            else if (c == '\b') {
                if (cmd_pos > 0) {
                    cmd_pos--;
                    fb_put_char('\b', 0xAAAAAA);
                }
            }
            else if (c >= ' ' && c <= '~') {
                if (cmd_pos < sizeof(cmd_buffer) - 1) {
                    cmd_buffer[cmd_pos++] = c;
                    fb_put_char(c, 0xAAAAAA);
                }
            }
        }
    }
}