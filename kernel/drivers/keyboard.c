#include <drivers/keyboard.h>
#include <drivers/serial.h>
#include <generic/irq.h>
#include <generic/io.h>
#include <drivers/fbtext.h>

#define KBD_BUFFER_SIZE 128

static char kbd_buffer[KBD_BUFFER_SIZE];
static volatile unsigned int kbd_head = 0;
static volatile unsigned int kbd_tail = 0;

static inline bool kbd_buffer_empty(void) {
    return kbd_head == kbd_tail;
}

static inline bool kbd_buffer_full(void) {
    return ((kbd_head + 1) % KBD_BUFFER_SIZE) == kbd_tail;
}

static void kbd_buffer_push(char c) {
    if (!kbd_buffer_full()) {
        kbd_buffer[kbd_head] = c;
        kbd_head = (kbd_head + 1) % KBD_BUFFER_SIZE;
    }
}

static char kbd_buffer_pop(void) {
    if (kbd_buffer_empty()) return 0;
    char c = kbd_buffer[kbd_tail];
    kbd_tail = (kbd_tail + 1) % KBD_BUFFER_SIZE;
    return c;
}

static const char ascii_base[128] = {
    0,   0,   '1',  '2',  '3',  '4',  '5',  '6',  '7',  '8',  '9',  '0',  '-',  '=',  '\b', '\t',
    'q', 'w', 'e',  'r',  't',  'y',  'u',  'i',  'o',  'p',  '[',  ']',  '\n', 0,    'a',  's',
    'd', 'f', 'g',  'h',  'j',  'k',  'l',  ';',  '\'', '`',  0,    '\\', 'z',  'x',  'c',  'v',
    'b', 'n', 'm',  ',',  '.',  '/',  0,    '*',  0,    ' ',  0,    0,    0,    0,    0,    0,
    0,   0,   0,    0,    0,    0,    0,    '7',  '8',  '9',  '-',  '4',  '5',  '6',  '+',  '1',
    '2', '3', '0',  '.',  0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,   0,   0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,   0,   0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0
};

static const char ascii_shift[128] = {
    0,   0,   '!',  '@',  '#',  '$',  '%',  '^',  '&',  '*',  '(',  ')',  '_',  '+',  '\b', '\t',
    'Q', 'W', 'E',  'R',  'T',  'Y',  'U',  'I',  'O',  'P',  '{',  '}',  '\n', 0,    'A',  'S',
    'D', 'F', 'G',  'H',  'J',  'K',  'L',  ':',  '"',  '~',  0,    '|',  'Z',  'X',  'C',  'V',
    'B', 'N', 'M',  '<',  '>',  '?',  0,    '*',  0,    ' ',  0,    0,    0,    0,    0,    0,
    0,   0,   0,    0,    0,    0,    0,    '7',  '8',  '9',  '-',  '4',  '5',  '6',  '+',  '1',
    '2', '3', '0',  '.',  0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,   0,   0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,   0,   0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0
};

static volatile bool left_shift = false;
static volatile bool right_shift = false;
static volatile bool caps_lock = false;

static inline bool is_shift_pressed(void) {
    return left_shift || right_shift;
}

static char get_char_from_scancode(uint8_t code) {
    const char* table = ascii_base;

    if (is_shift_pressed()) {
        table = ascii_shift;
    }

    char ch = (code < 128) ? table[code] : 0;

    if (caps_lock && ch >= 'a' && ch <= 'z') {
        ch -= 32;
    }
    else if (caps_lock && ch >= 'A' && ch <= 'Z' && is_shift_pressed()) {
        ch += 32;
    }

    return ch;
}

void keyboard_handler(void) {
    uint8_t scancode = inb(0x60);
    bool is_break = (scancode & 0x80) != 0;
    uint8_t code = scancode & 0x7F;

    if (is_break) {
        if (code == 0x2A) left_shift = false;
        if (code == 0x36) right_shift = false;
    } else {
        if (code == 0x2A) left_shift = true;
        else if (code == 0x36) right_shift = true;
        else if (code == 0x3A) caps_lock = !caps_lock;
        else {
            char ch = get_char_from_scancode(code);
            kbd_buffer_push(ch);
        }
    }

    irq_eoi();
}

extern void keyboard_isr(void);

void keyboard_init(void) {
    irq_set(
        1,
        0x21,
        false,
        false,
        IRQ_DELMODE_FIXED,
        0
    );
    serial_puts("PS/2 keyboard driver initialized\n");
}

bool keyboard_has_data(void) {
    return !kbd_buffer_empty();
}

char keyboard_get_char(void) {
    return kbd_buffer_pop();
}