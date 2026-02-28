#include <klib/string.h>
#include <klib/memory.h>
#include <drivers/fbtext.h>
#include <drivers/keyboard.h>
#include <drivers/serial.h>
#include <colors.h>
#include <generic/time.h>
#include <generic/usermode.h>

#define INPUT_BUFFER_SIZE 128

static char input_buffer[INPUT_BUFFER_SIZE];
static int input_index = 0;

static void print_prompt(void) {
    fb_print("sh> ", 0x7FFFD4);
}

static void clear_input_buffer(void) {
    memset(input_buffer, 0, INPUT_BUFFER_SIZE);
    input_index = 0;
}

static void execute_command(const char* cmd) {
    if (!cmd || cmd[0] == '\0') return;

    if (strcmp(cmd, "help") == 0) {
        fb_print("Available commands: help, clear, about, time, panic, usermode, halt\n", 0x00FF00);
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
    else if (strcmp(cmd, "usermode") == 0) {
        fb_print("Entering usermode - ring 3..\n", 0xFFAA00);
        serial_puts("Entering usermode - ring 3..\n");
        enter_usermode();
    }
    else {
        fb_print("Unknown command: ", 0xAA0000);
        fb_print(cmd, 0xAAAAAA);
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
    else if (c == '\b') {
        if (input_index > 0) {
            input_index--;
            input_buffer[input_index] = 0;
            fb_print("\b", 0xFFFFFF);
        }
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

        asm volatile("pause");
    }
}