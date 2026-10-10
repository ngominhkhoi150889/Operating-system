#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include "libk.h"

static void test_memset(void) {
    unsigned char buffer[4] = {0, 0, 0, 0};
    assert(k_memset(buffer, 0xA5, sizeof buffer) == buffer);
    for (size_t i = 0; i < sizeof buffer; ++i) {
        assert(buffer[i] == 0xA5);
    }
}

static void test_memcpy(void) {
    const char source[] = "KhoiOS";
    char target[sizeof source] = {0};
    assert(k_memcpy(target, source, sizeof source) == target);
    assert(k_strcmp(target, "KhoiOS") == 0);
}

static void test_memmove(void) {
    char text[] = "abcdef";
    k_memmove(text + 2, text, 4);
    assert(k_strcmp(text, "ababcd") == 0);

    k_memmove(text, text + 2, 4);
    assert(k_strncmp(text, "abcd", 4) == 0);
}

static void test_strings(void) {
    assert(k_strlen("") == 0);
    assert(k_strlen("KhoiOS") == 6);
    assert(k_strcmp("abc", "abc") == 0);
    assert(k_strcmp("abc", "abd") < 0);
    assert(k_strcmp("abd", "abc") > 0);
    assert(k_strncmp("kernel", "key", 2) == 0);
    assert(k_strncmp("kernel", "key", 3) != 0);
    assert(k_strncmp("abc", "xyz", 0) == 0);
}

int main(void) {
    test_memset();
    test_memcpy();
    test_memmove();
    test_strings();
    puts("KhoiOS libk tests: PASS");
    return 0;
}
