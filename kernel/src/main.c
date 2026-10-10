#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <limine.h>

__attribute__((used, section(".limine_requests")))
static volatile uint64_t base_revision[] = LIMINE_BASE_REVISION(6);

__attribute__((used, section(".limine_requests")))
static volatile struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST_ID,
    .revision = 0
};

__attribute__((used, section(".limine_requests_start")))
static volatile uint64_t requests_start[] = LIMINE_REQUESTS_START_MARKER;

__attribute__((used, section(".limine_requests_end")))
static volatile uint64_t requests_end[] = LIMINE_REQUESTS_END_MARKER;

_Noreturn static void halt_cpu(void) {
    for (;;) {
        __asm__ volatile ("cli; hlt");
    }
}

static uint32_t channel(uint8_t value, uint8_t size, uint8_t shift) {
    uint64_t max = (((uint64_t)1 << size) - 1);
    return (uint32_t)(((uint64_t)value * max / 255) << shift);
}

static uint32_t rgb(struct limine_framebuffer *fb,
                    uint8_t r, uint8_t g, uint8_t b) {
    return channel(r, fb->red_mask_size, fb->red_mask_shift)
         | channel(g, fb->green_mask_size, fb->green_mask_shift)
         | channel(b, fb->blue_mask_size, fb->blue_mask_shift);
}

static void pixel(struct limine_framebuffer *fb, size_t x, size_t y,
                  uint32_t color) {
    if (x < fb->width && y < fb->height) {
        volatile uint32_t *p = (volatile uint32_t *)fb->address;
        p[y * (fb->pitch / 4) + x] = color;
    }
}

static void rect(struct limine_framebuffer *fb, size_t x, size_t y,
                 size_t w, size_t h, uint32_t color) {
    for (size_t yy = y; yy < y + h && yy < fb->height; yy++) {
        for (size_t xx = x; xx < x + w && xx < fb->width; xx++) {
            pixel(fb, xx, yy, color);
        }
    }
}

/* Minimal 5x7 glyphs for the KhoiOS boot banner. */
static const uint8_t *glyph(char c) {
    static const uint8_t K[7] = {17,18,20,24,20,18,17};
    static const uint8_t H[7] = {17,17,17,31,17,17,17};
    static const uint8_t O[7] = {14,17,17,17,17,17,14};
    static const uint8_t I[7] = {14,4,4,4,4,4,14};
    static const uint8_t S[7] = {15,16,16,14,1,1,30};
    static const uint8_t B[7] = {30,17,17,30,17,17,30};
    static const uint8_t T[7] = {31,4,4,4,4,4,4};
    static const uint8_t space[7] = {0,0,0,0,0,0,0};
    switch (c) {
        case 'K': return K; case 'H': return H; case 'O': return O;
        case 'I': return I; case 'S': return S; case 'B': return B;
        case 'T': return T; default: return space;
    }
}

static void text5x7(struct limine_framebuffer *fb, size_t x, size_t y,
                    const char *s, uint32_t color, size_t scale) {
    for (; *s; s++, x += 6 * scale) {
        const uint8_t *g = glyph(*s);
        for (size_t row = 0; row < 7; row++) {
            for (size_t col = 0; col < 5; col++) {
                if (g[row] & (1u << (4 - col))) {
                    rect(fb, x + col * scale, y + row * scale,
                         scale, scale, color);
                }
            }
        }
    }
}

void kmain(void) {
    if (!LIMINE_BASE_REVISION_SUPPORTED(base_revision)) {
        halt_cpu();
    }

    if (framebuffer_request.response == NULL ||
        framebuffer_request.response->framebuffer_count == 0) {
        halt_cpu();
    }

    struct limine_framebuffer *fb =
        framebuffer_request.response->framebuffers[0];

    if (fb->memory_model != LIMINE_FRAMEBUFFER_RGB || fb->bpp != 32) {
        halt_cpu();
    }

    uint32_t bg = rgb(fb, 12, 18, 35);
    uint32_t accent = rgb(fb, 55, 170, 255);
    uint32_t white = rgb(fb, 240, 245, 255);
    rect(fb, 0, 0, fb->width, fb->height, bg);

    size_t scale = fb->width >= 800 ? 8 : 4;
    size_t banner_width = 14 * 6 * scale;
    size_t x = fb->width > banner_width ? (fb->width - banner_width) / 2 : 8;
    size_t y = fb->height / 2 > 40 ? fb->height / 2 - 40 : 8;

    rect(fb, x - 18, y - 20, banner_width + 36, 7 * scale + 40,
         rgb(fb, 22, 31, 55));
    rect(fb, x - 18, y - 20, banner_width + 36, 4, accent);
    text5x7(fb, x, y, "KHOIOS BOOT OK", white, scale);

    size_t bar_y = y + 7 * scale + 12;
    rect(fb, x, bar_y, banner_width, 3, accent);

    /* The first milestone is a boot screen; keep the CPU idle afterward. */
    halt_cpu();
}
