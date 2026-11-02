#pragma once
#include <stdint.h>

struct __attribute__((packed)) tss_entry {
    uint32_t reserved0;
    uint64_t rsp0;       // Stack for ring 0
    uint64_t rsp1;
    uint64_t rsp2;
    uint64_t reserved1;
    uint64_t ist[7];
    uint64_t reserved2;
    uint16_t reserved3;
    uint16_t iomap_base;
};

static struct tss_entry tss;

void init_tss();