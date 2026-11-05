#include "kmalloc.h"
#include "mm/vmm.h"
#include "mm/pmm.h"
#include "klib/string.h"
#include <lib/stdio.h>

static uint64_t kheap_current_end = KHEAP_START;
static block_header_t* free_list = NULL;

static block_header_t* kheap_extend(size_t size) {
    size_t pages = (size + sizeof(block_header_t) + 4095) / 4096;
    uint64_t* pml4 = get_pml4_address();
    
    uint64_t new_block_vaddr = kheap_current_end;

    for(size_t i = 0; i < pages; i++) {
        uint64_t phys = pmm_alloc_page();
        if(!phys) {
            kprintf("CRITICAL: kmalloc failed to extend heap - Out of Physical Memory\n");
            return NULL;
        }
        vmm_map(pml4, kheap_current_end, phys, VMM_PRESENT | VMM_WRITABLE);
        kheap_current_end += 4096;
    }

    block_header_t* new_block = (block_header_t*)new_block_vaddr;
    new_block->size = (pages * 4096) - sizeof(block_header_t);
    new_block->is_free = 1;
    new_block->next = NULL;

    return new_block;
}

void* kmalloc(size_t size) {
    if(size == 0) return NULL;

    // Aligning
    size = (size + 7) & ~7;

    if(free_list == NULL) {
        free_list = kheap_extend(size);
        if (!free_list) return NULL;
    }

    block_header_t* current = free_list;
    block_header_t* last = current;

    while(current) {
        if(current->is_free && current->size >= size) {
            // if the block too big, split it
            if(current->size > size + sizeof(block_header_t) + 8) {
                block_header_t* split = (block_header_t*)((uintptr_t)current + sizeof(block_header_t) + size);
                split->size = current->size - size - sizeof(block_header_t);
                split->is_free = 1;
                split->next = current->next;

                current->size = size;
                current->next = split;
            }
            current->is_free = 0;
            return (void*)((uintptr_t)current + sizeof(block_header_t));
        }
        last = current;
        current = current->next;
    }

    block_header_t* new_block = kheap_extend(size);
    if(!new_block) return NULL;

    last->next = new_block;

    new_block->is_free = 0;
    return (void*)((uintptr_t)new_block + sizeof(block_header_t));
}
