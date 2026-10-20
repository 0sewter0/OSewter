#include <stdint.h>
#include <stddef.h>
#include "limine.h"

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

    // Get the main pointer to the first display
    struct limine_framebuffer *fb = framebuffer_request.response->framebuffers[0];

    // Beautiful blue filling
    for (uint64_t y = 0; y < fb->height; y++) {
        for (uint64_t x = 0; x < fb->width; x++) {
            draw_pixel(fb, x, y, 0x002B36);
        }
    }

    // Draw a white square to test
    uint64_t start_x = fb->width / 2 - 50;
    uint64_t start_y = fb->height / 2 - 50;
    for (uint64_t y = start_y; y < start_y + 100; y++) {
        for (uint64_t x = start_x; x < start_x + 100; x++) {
            draw_pixel(fb, x, y, 0xFFFFFF);
        }
    }

    for (;;) { __asm__("hlt"); }
}
