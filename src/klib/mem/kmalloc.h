#pragma once
#include <stddef.h>

#define KHEAP_START 0xFFFF900000000000ULL

typedef struct block_header {
    size_t size;
    int is_free;
    struct block_header* next;
} block_header_t;

void* kmalloc(size_t size);
