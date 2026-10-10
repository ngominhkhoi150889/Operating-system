#include <stdint.h>
#include "libk.h"

void *k_memset(void *dest, int value, size_t count) {
    unsigned char *out = (unsigned char *)dest;
    for (size_t i = 0; i < count; ++i) {
        out[i] = (unsigned char)value;
    }
    return dest;
}

void *k_memcpy(void *dest, const void *src, size_t count) {
    unsigned char *out = (unsigned char *)dest;
    const unsigned char *in = (const unsigned char *)src;
    for (size_t i = 0; i < count; ++i) {
        out[i] = in[i];
    }
    return dest;
}

void *k_memmove(void *dest, const void *src, size_t count) {
    unsigned char *out = (unsigned char *)dest;
    const unsigned char *in = (const unsigned char *)src;

    if (out == in || count == 0) {
        return dest;
    }

    if ((uintptr_t)out < (uintptr_t)in) {
        for (size_t i = 0; i < count; ++i) {
            out[i] = in[i];
        }
    } else {
        for (size_t i = count; i > 0; --i) {
            out[i - 1] = in[i - 1];
        }
    }
    return dest;
}

size_t k_strlen(const char *str) {
    size_t length = 0;
    while (str[length] != '\0') {
        ++length;
    }
    return length;
}

int k_strcmp(const char *left, const char *right) {
    while (*left != '\0' && *left == *right) {
        ++left;
        ++right;
    }
    return (unsigned char)*left - (unsigned char)*right;
}

int k_strncmp(const char *left, const char *right, size_t count) {
    for (size_t i = 0; i < count; ++i) {
        unsigned char a = (unsigned char)left[i];
        unsigned char b = (unsigned char)right[i];
        if (a != b || a == '\0') {
            return (int)a - (int)b;
        }
    }
    return 0;
}
