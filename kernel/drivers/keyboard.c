#include <drivers/keyboard.h>
#include <drivers/serial.h>
#include <generic/irq.h>
#include <generic/io.h>

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

static volatile bool     key_pressed    = false;
static volatile uint8_t  last_scancode  = 0;
static volatile bool     left_shift     = false;
static volatile bool     right_shift    = false;
static volatile bool     caps_lock      = false;

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
        switch (code) {
            case 0x2A:
                left_shift = false;
                break;
            case 0x36:
                right_shift = false;
                break;
        }
    }
    else {
        switch (code) {
            case 0x2A:
                left_shift = true;
                break;
            case 0x36:
                right_shift = true;
                break;

            case 0x3A:
                caps_lock = !caps_lock;
                break;

            default:
                last_scancode = scancode;
                key_pressed = true;
                break;
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
    return key_pressed;
}

uint8_t keyboard_get_scancode(void) {
    key_pressed = false;
    return last_scancode;
}

char keyboard_get_char(void) {
    if (!key_pressed) {
        return 0;
    }

    uint8_t sc = last_scancode;
    bool is_make = (sc < 0x80);

    if (!is_make) {
        key_pressed = false;
        return 0;
    }

    uint8_t code = sc & 0x7F;
    char ch = get_char_from_scancode(code);

    key_pressed = false;

    return ch;
}