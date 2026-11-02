#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>
#include <drivers/serial.h>

static void kprint_hex64(uint64_t value) {
    const char* hex_digits = "0123456789abcdef";
    
    serial_putc('0');
    serial_putc('x');
    for(int i = 15; i >= 0; i--) {
        uint8_t nibble = (value >> (i * 4)) & 0xF;
        serial_putc(hex_digits[nibble]);
    }
}

static void kprint_string(const char* str) {
    if(!str) str = "(null)";
    while(*str) {
        serial_putc(*str++);
    }
}

void kprintf(const char* format, ...) {
    va_list args;
    va_start(args, format);

    while(*format) {
        if(*format != '%') {
            serial_putc(*format);
            format++;
            continue;
        }
        format++;
        switch(*format) {
            case 'c': {
                char c = (char)va_arg(args, int);
                serial_putc(c);
                break;
            }
            case 's': {
                const char* s = va_arg(args, const char*);
                kprint_string(s);
                break;
            }
            case 'x':
            case 'p': {
                uint64_t val = va_arg(args, uint64_t);
                kprint_hex64(val);
                break;
            }
            case '%': {
                serial_putc('%');
                break;
            }
            default: {
                serial_putc('%');
                serial_putc(*format);
                break;
            }
        }
        format++;
    }

    va_end(args);
}
