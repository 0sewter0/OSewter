#include <stdint.h>
#include <stddef.h>

#include "limine.h"
#include "arch/x86_64/gdt/gdt.h"
#include "arch/x86_64/idt/idt.h"
#include "mm/pmm.h"
#include "drivers/serial.h"

// Setting the base protocol revision
__attribute__((used, section(".limine_requests")))
static volatile uint64_t limine_base_revision[] = { 0xf4485f52dd2bb830, 0x5812d34d590f6e5f, 0 };

// We issue a request to initialize the graphics framebuffer.
__attribute__((used, section(".limine_requests")))
static volatile struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST,
    .revision = 0
};

void draw_pixel(struct limine_framebuffer *fb, uint64_t x, uint64_t y, uint32_t color) {
    uint32_t *pixel_address = (uint32_t *)((uint8_t *)fb->address + y * fb->pitch + x * (fb->bpp / 8));
    *pixel_address = color;
}

// Long mode
void _start(void) {
    if (framebuffer_request.response == NULL || framebuffer_request.response->framebuffer_count < 1) {
        for (;;) { __asm__("hlt"); }
    }

    gdt_init();
    idt_init();
    pmm_init();

    serial_init();
    serial_print("Serial logging driver initialized inside the terminal\n");

    serial_print("Testing IDT: Triggering Division by Zero Fault...\n");
    volatile int a = 5;
    volatile int b = 0;
    volatile int c = a / b; 
    (void)c;

    for (;;) { __asm__("hlt"); }
}
