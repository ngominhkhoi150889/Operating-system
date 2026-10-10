#ifndef KHOIOS_LIBK_H
#define KHOIOS_LIBK_H

#include <stddef.h>

void *k_memset(void *dest, int value, size_t count);
void *k_memcpy(void *dest, const void *src, size_t count);
void *k_memmove(void *dest, const void *src, size_t count);
size_t k_strlen(const char *str);
int k_strcmp(const char *left, const char *right);
int k_strncmp(const char *left, const char *right, size_t count);

#endif
