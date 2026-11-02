#include <stdint.h>
#include <stddef.h>
#include "vmm.h"
#include "pmm.h"
#include <klib/string.h>
#include "limine.h" 
#include <drivers/serial.h>

extern volatile struct limine_hhdm_request hhdm_request;

static inline void* phys_to_virt(uint64_t phys) {
    uint64_t hhdm_offset = hhdm_request.response->offset;
    return (void*)(phys + hhdm_offset);
}

static inline uint64_t virt_to_phys(void* virt) {
    uint64_t hhdm_offset = hhdm_request.response->offset;
    return (uint64_t)virt - hhdm_offset;
}

static uint64_t* get_next_table(uint64_t* current_table_virt, size_t index, uint64_t flags) {
    uint64_t entry = current_table_virt[index];
    
    if(!(entry & VMM_PRESENT)) {
        uint64_t new_table_phys = pmm_alloc_page(); 
        if (!new_table_phys) return NULL;

        uint64_t* new_table_virt = (uint64_t*)phys_to_virt(new_table_phys);

        memset(new_table_virt, 0, PAGE_SIZE);

        current_table_virt[index] = new_table_phys | VMM_PRESENT | flags;
        
        return new_table_virt;
    }

    uint64_t next_table_phys = entry & VMM_ADDR_MASK;
    return (uint64_t*)phys_to_virt(next_table_phys);
}

void vmm_map(uint64_t* pml4_virt, uint64_t virt, uint64_t phys, uint64_t flags) {
    size_t pml4_idx = (virt >> 39) & 0x1FF;
    size_t pdpt_idx = (virt >> 30) & 0x1FF;
    size_t pd_idx   = (virt >> 21) & 0x1FF;
    size_t pt_idx   = (virt >> 12) & 0x1FF;

    // Pass flags (e.g., VMM_WRITABLE | VMM_USER) so that intermediate tables inherit them
    uint64_t* pdpt_virt = get_next_table(pml4_virt, pml4_idx, flags);
    uint64_t* pd_virt   = get_next_table(pdpt_virt, pdpt_idx, flags);
    uint64_t* pt_virt   = get_next_table(pd_virt,   pd_idx,   flags);

    pt_virt[pt_idx] = (phys & VMM_ADDR_MASK) | VMM_PRESENT | flags;

    // Flush the processor cache (TLB) for the modified address
    __asm__ volatile("invlpg (%0)" :: "r"(virt) : "memory");
}

void vmm_init(void) {
    // Allocate a physical page for the new root PML4 table
    uint64_t new_pml4_phys = pmm_alloc_page();
    uint64_t* new_pml4_virt = (uint64_t*)phys_to_virt(new_pml4_phys);
    memset(new_pml4_virt, 0, PAGE_SIZE);

    uint64_t old_pml4_phys;
    __asm__ volatile("mov %%cr3, %0" : "=r"(old_pml4_phys));
    uint64_t* old_pml4_virt = (uint64_t*)phys_to_virt(old_pml4_phys);

    // Indices 256 through 511 correspond to the upper virtual addresses (kernel and HHDM)
    for(size_t i = 256; i < 512; i++) {
        new_pml4_virt[i] = old_pml4_virt[i];
    }

    __asm__ volatile("mov %0, %%cr3" :: "r"(new_pml4_phys) : "memory");
}
