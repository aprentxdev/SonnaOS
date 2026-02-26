#include <klib/string.h>
#include <klib/memory.h>
#include <drivers/fbtext.h>
#include <drivers/keyboard.h>
#include <drivers/serial.h>
#include <arch/x86_64/apic.h>
#include <stopwatch.h>
#include <colors.h>

#define INPUT_BUFFER_SIZE 128

static char input_buffer[INPUT_BUFFER_SIZE];
static int input_index = 0;

static void print_prompt(void) {
    fb_print("sh>", 0x7FFFD4);
}

static void clear_input_buffer(void) {
    memset(input_buffer, 0, INPUT_BUFFER_SIZE);
    input_index = 0;
}

static void execute_command(const char* cmd) {
    if (!cmd || cmd[0] == '\0') return;

    if (strcmp(cmd, "help") == 0) {
        fb_print("Available commands: help, clear, about, time, panic, halt\n", COL_INFO);
    }
    else if (strcmp(cmd, "clear") == 0) { 
        fb_clear();
    }
    else if (strcmp(cmd, "about") == 0) {
        fb_print("Estella kernelspace shell\n", COL_INFO);
        fb_print("\n", 0);
    }
    else if (strcmp(cmd, "time") == 0) {
        fb_print("later...\n", COL_INFO);
    }
    else if (strcmp(cmd, "panic") == 0) {
        fb_print("Triggering kernel panic...\n", COL_INFO);
        asm volatile("ud2");
    }
    else if (strcmp(cmd, "halt") == 0) {
        fb_print("Halting system...\n", COL_INFO);
        asm volatile("cli; hlt");
        while (1) asm("hlt");
    }
    else {
        fb_print("Unknown command: ", COL_INFO);
        fb_print(cmd, COL_INFO);
        fb_print("\n", 0);
    }
}

static void handle_input_char(char c) {
    if (c == '\n') {
        input_buffer[input_index] = '\0';
        fb_print("\n", 0);
        execute_command(input_buffer);
        clear_input_buffer();
        print_prompt();
    }
    else if (c >= 32 && c <= 126) {
        if (input_index < INPUT_BUFFER_SIZE - 1) {
            input_buffer[input_index++] = c;
            char str[2] = {c, 0};
            fb_print(str, 0xFFFFFF);
        }
    }
}

void launch_shell(void) {
    clear_input_buffer();
    fb_print("Welcome to Estella kernel shell \n", COL_SUCCESS_INIT);
    fb_print("Type 'help' for commands.\n\n", COL_SUCCESS_INIT);

    print_prompt();
    while (1) {
        if (keyboard_has_data()) {
            char c = keyboard_get_char();
            if (c) {
                handle_input_char(c);
            }
        }

        uint64_t now = timer_get_tsc();
        stopwatch_update(now, tsc_frequency_hz);

        asm volatile("pause");
    }
}