#pragma once
#include <stdint.h>
#include <stddef.h>

#define PAGE_SIZE 4096

// Page flags
#define VMM_PRESENT  (1ULL << 0) 
#define VMM_WRITABLE (1ULL << 1)
#define VMM_USER     (1ULL << 2)
#define VMM_NX       (1ULL << 63) // No-Execute

// Mask for extracting the virtual address from entries (bits 12–51)
#define VMM_ADDR_MASK 0x000FFFFFFFFFF000ULL

typedef uint64_t pml4_table_t[512];
typedef uint64_t page_table_t[512];

void vmm_init(void);
void vmm_map(uint64_t* pml4, uint64_t virt, uint64_t phys, uint64_t flags);
