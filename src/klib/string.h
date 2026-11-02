#pragma once
#include <stddef.h>
#include <stdint.h>

void* memset(void* dest, int ch, size_t count);
void* memcpy(void* restrict dest, const void* restrict src, size_t count);
void* memmove(void* dest, const void* src, size_t count);
