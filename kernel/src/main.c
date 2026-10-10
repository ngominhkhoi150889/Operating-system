#include <stdint.h>
#include <stddef.h>
#include "libk.h"

#define MULTIBOOT2_BOOT_MAGIC 0x36d76289u
#define MULTIBOOT2_TAG_END 0u
#define MULTIBOOT2_TAG_MODULE 3u
#define MULTIBOOT2_TAG_FRAMEBUFFER 8u
#define MULTIBOOT2_FRAMEBUFFER_RGB 1u

struct multiboot2_tag {
    uint32_t type;
    uint32_t size;
};

struct multiboot2_tag_module {
    uint32_t type;
    uint32_t size;
    uint32_t module_start;
    uint32_t module_end;
    char command_line[];
} __attribute__((packed));

struct os_release_info {
    char name[64];
    char version[32];
    char architecture[32];
    char platform[32];
};

static struct os_release_info os_release;

struct multiboot2_tag_framebuffer {
    uint32_t type;
    uint32_t size;
    uint64_t address;
    uint32_t pitch;
    uint32_t width;
    uint32_t height;
    uint8_t bpp;
    uint8_t framebuffer_type;
    uint16_t reserved;
    uint8_t red_position;
    uint8_t red_mask_size;
    uint8_t green_position;
    uint8_t green_mask_size;
    uint8_t blue_position;
    uint8_t blue_mask_size;
} __attribute__((packed));

struct framebuffer {
    volatile uint32_t *pixels;
    size_t width;
    size_t height;
    size_t pitch_pixels;
    uint8_t red_position;
    uint8_t red_size;
    uint8_t green_position;
    uint8_t green_size;
    uint8_t blue_position;
    uint8_t blue_size;
};

_Noreturn static void halt_cpu(void) {
    for (;;) {
        __asm__ volatile ("cli; hlt");
    }
}

static uint32_t channel(uint8_t value, uint8_t size, uint8_t position) {
    if (size == 0 || size > 8 || position >= 32 ||
        (uint16_t)size + position > 32) {
        return 0;
    }
    uint64_t max = (((uint64_t)1 << size) - 1);
    return (uint32_t)(((uint64_t)value * max / 255) << position);
}

static uint32_t rgb(const struct framebuffer *fb,
                    uint8_t r, uint8_t g, uint8_t b) {
    return channel(r, fb->red_size, fb->red_position)
         | channel(g, fb->green_size, fb->green_position)
         | channel(b, fb->blue_size, fb->blue_position);
}

static void pixel(struct framebuffer *fb, size_t x, size_t y,
                  uint32_t color) {
    if (x < fb->width && y < fb->height) {
        fb->pixels[y * fb->pitch_pixels + x] = color;
    }
}

static void rect(struct framebuffer *fb, size_t x, size_t y,
                 size_t w, size_t h, uint32_t color) {
    size_t end_y = y + h;
    size_t end_x = x + w;
    if (end_y < y || end_y > fb->height) end_y = fb->height;
    if (end_x < x || end_x > fb->width) end_x = fb->width;
    for (size_t yy = y; yy < end_y; ++yy) {
        for (size_t xx = x; xx < end_x; ++xx) {
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
    static const uint8_t A[7] = {14,17,17,31,17,17,17};
    static const uint8_t N[7] = {17,25,25,21,19,19,17};
    static const uint8_t M[7] = {17,27,21,21,17,17,17};
    static const uint8_t dash[7] = {0,0,0,31,0,0,0};
    static const uint8_t T[7] = {31,4,4,4,4,4,4};
    static const uint8_t space[7] = {0,0,0,0,0,0,0};
    switch (c) {
        case 'K': return K;
        case 'H': return H;
        case 'O': return O;
        case 'I': return I;
        case 'S': return S;
        case 'B': return B;
        case 'A': return A;
        case 'N': return N;
        case 'M': return M;
        case '-': return dash;
        case 'T': return T;
        default: return space;
    }
}

static void text5x7(struct framebuffer *fb, size_t x, size_t y,
                    const char *s, uint32_t color, size_t scale) {
    for (; *s; ++s, x += 6 * scale) {
        const uint8_t *g = glyph(*s);
        for (size_t row = 0; row < 7; ++row) {
            for (size_t col = 0; col < 5; ++col) {
                if (g[row] & (1u << (4 - col))) {
                    rect(fb, x + col * scale, y + row * scale,
                         scale, scale, color);
                }
            }
        }
    }
}

static struct multiboot2_tag_framebuffer *find_framebuffer(uintptr_t info_addr) {
    if (info_addr == 0) {
        return NULL;
    }

    const uint8_t *base = (const uint8_t *)info_addr;
    uint32_t total_size = *(const uint32_t *)base;
    if (total_size < 16 || total_size > 1024 * 1024) {
        return NULL;
    }

    size_t offset = 8;
    while (offset + sizeof(struct multiboot2_tag) <= total_size) {
        const struct multiboot2_tag *tag =
            (const struct multiboot2_tag *)(base + offset);
        if (tag->size < sizeof(*tag) || tag->size > total_size - offset) {
            return NULL;
        }
        if (tag->type == MULTIBOOT2_TAG_END) {
            break;
        }
        if (tag->type == MULTIBOOT2_TAG_FRAMEBUFFER &&
            tag->size >= sizeof(struct multiboot2_tag_framebuffer)) {
            return (struct multiboot2_tag_framebuffer *)(uintptr_t)tag;
        }
        offset += (tag->size + 7u) & ~7u;
    }
    return NULL;
}


static int key_is(const char *key, size_t key_len, const char *expected) {
    return k_strlen(expected) == key_len &&
           k_strncmp(key, expected, key_len) == 0;
}

static void copy_release_value(char *out, size_t capacity,
                               const char *value, size_t length) {
    if (capacity == 0) return;
    if (length > 0 && value[0] == '"') {
        ++value;
        --length;
        if (length > 0 && value[length - 1] == '"') --length;
    }
    if (length >= capacity) length = capacity - 1;
    k_memcpy(out, value, length);
    out[length] = '\0';
}

static int parse_os_release(const char *data, size_t length) {
    k_memset(&os_release, 0, sizeof(os_release));
    size_t pos = 0;
    while (pos < length) {
        size_t line_start = pos;
        while (pos < length && data[pos] != '\n' && data[pos] != '\r') ++pos;
        size_t line_end = pos;
        while (pos < length && (data[pos] == '\n' || data[pos] == '\r')) ++pos;
        if (line_end == line_start || data[line_start] == '#') continue;

        size_t equal = line_start;
        while (equal < line_end && data[equal] != '=') ++equal;
        if (equal == line_end) continue;
        const char *key = data + line_start;
        size_t key_len = equal - line_start;
        const char *value = data + equal + 1;
        size_t value_len = line_end - equal - 1;

        if (key_is(key, key_len, "NAME"))
            copy_release_value(os_release.name, sizeof(os_release.name), value, value_len);
        else if (key_is(key, key_len, "VERSION_ID"))
            copy_release_value(os_release.version, sizeof(os_release.version), value, value_len);
        else if (key_is(key, key_len, "ARCHITECTURE"))
            copy_release_value(os_release.architecture, sizeof(os_release.architecture), value, value_len);
        else if (key_is(key, key_len, "PLATFORM"))
            copy_release_value(os_release.platform, sizeof(os_release.platform), value, value_len);
    }

    return os_release.name[0] != '\0' &&
           os_release.version[0] != '\0' &&
           os_release.architecture[0] != '\0' &&
           os_release.platform[0] != '\0';
}

static int load_os_release(uintptr_t info_addr) {
    if (info_addr == 0) return 0;
    const uint8_t *base = (const uint8_t *)info_addr;
    uint32_t total_size = *(const uint32_t *)base;
    if (total_size < 16 || total_size > 1024 * 1024) return 0;

    size_t offset = 8;
    while (offset + sizeof(struct multiboot2_tag) <= total_size) {
        const struct multiboot2_tag *tag =
            (const struct multiboot2_tag *)(base + offset);
        if (tag->size < sizeof(*tag) || tag->size > total_size - offset) return 0;
        if (tag->type == MULTIBOOT2_TAG_END) break;
        if (tag->type == MULTIBOOT2_TAG_MODULE &&
            tag->size >= sizeof(struct multiboot2_tag_module)) {
            const struct multiboot2_tag_module *module =
                (const struct multiboot2_tag_module *)tag;
            if (module->module_end > module->module_start &&
                module->module_end <= 0x100000000ull &&
                module->module_start < 0x100000000ull) {
                size_t length = module->module_end - module->module_start;
                const char *cmd = module->command_line;
                if (k_strncmp(cmd, "os-release", 10) == 0 &&
                    (cmd[10] == '\0' || cmd[10] == ' ')) {
                    return parse_os_release(
                        (const char *)(uintptr_t)module->module_start, length);
                }
            }
        }
        offset += (tag->size + 7u) & ~7u;
    }
    return 0;
}

void kmain(uint64_t info_addr, uint64_t boot_magic) {
    if (boot_magic != MULTIBOOT2_BOOT_MAGIC || !load_os_release((uintptr_t)info_addr) ||
        k_strcmp(os_release.name, "Banh-mi-OS") != 0 ||
        k_strcmp(os_release.architecture, "x86_64") != 0) {
        halt_cpu();
    }

    struct multiboot2_tag_framebuffer *tag =
        find_framebuffer((uintptr_t)info_addr);
    if (tag == NULL || tag->framebuffer_type != MULTIBOOT2_FRAMEBUFFER_RGB ||
        tag->bpp != 32 || tag->pitch < tag->width * 4u ||
        tag->width == 0 || tag->height == 0) {
        halt_cpu();
    }

    /* The bootstrap identity-maps the first 4 GiB of physical memory. */
    uint64_t bytes = (uint64_t)tag->pitch * tag->height;
    if (tag->address >= 0x100000000ull ||
        bytes > 0x100000000ull - tag->address) {
        halt_cpu();
    }

    struct framebuffer fb = {
        .pixels = (volatile uint32_t *)(uintptr_t)tag->address,
        .width = tag->width,
        .height = tag->height,
        .pitch_pixels = tag->pitch / 4,
        .red_position = tag->red_position,
        .red_size = tag->red_mask_size,
        .green_position = tag->green_position,
        .green_size = tag->green_mask_size,
        .blue_position = tag->blue_position,
        .blue_size = tag->blue_mask_size
    };

    uint32_t bg = rgb(&fb, 12, 18, 35);
    uint32_t accent = rgb(&fb, 55, 170, 255);
    uint32_t white = rgb(&fb, 240, 245, 255);
    rect(&fb, 0, 0, fb.width, fb.height, bg);

    size_t scale = fb.width >= 800 ? 8 : 4;
    size_t banner_width = 10 * 6 * scale;
    size_t x = fb.width > banner_width ? (fb.width - banner_width) / 2 : 8;
    size_t y = fb.height / 2 > 40 ? fb.height / 2 - 40 : 8;

    rect(&fb, x >= 18 ? x - 18 : 0, y >= 20 ? y - 20 : 0,
         banner_width + 36, 7 * scale + 40, rgb(&fb, 22, 31, 55));
    rect(&fb, x >= 18 ? x - 18 : 0, y >= 20 ? y - 20 : 0,
         banner_width + 36, 4, accent);
    text5x7(&fb, x, y, "BANH-MI-OS", white, scale);

    size_t bar_y = y + 7 * scale + 12;
    rect(&fb, x, bar_y, banner_width, 3, accent);

    halt_cpu();
}
