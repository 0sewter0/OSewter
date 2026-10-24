#include <stdint.h>
#include <stddef.h>
#include "pmm.h"
#include "limine.h"


__attribute__((used, section(".limine_requests")))
static volatile struct limine_memmap_request memmap_request = {
    .id = LIMINE_MEMMAP_REQUEST,
    .revision = 0
};

static volatile struct limine_hhdm_request hhdm_request = {
    .id = LIMINE_HHDM_REQUEST,
    .revision = 0
};

uint64_t* pmm_bitmap = NULL;
size_t pmm_total_pages = 0;
size_t pmm_bitmap_size = 0;
size_t pmm_bitmap_size_bytes = 0;
size_t pmm_last_word = 0;

static inline void bitmap_set(size_t bit) {
    pmm_bitmap[bit / 64] |= (1 << (bit % 64));
}

static inline void bitmap_clear(size_t bit) {
    pmm_bitmap[bit / 64] &= ~(1 << (bit % 64));
}

uintptr_t pmm_alloc_page(void) {
    for(size_t i = pmm_last_word; i < pmm_total_pages; i++) {
        if(pmm_bitmap[i] == 0xFFFFFFFFFFFFFFFFULL) {
            continue;
        }

        uint64_t inverted = ~pmm_bitmap[i];
        int bit_index = __builtin_ctzll(inverted);

        size_t total_bit_index = i * 64 + bit_index;
        

        pmm_bitmap[i] |= (1ULL << bit_index);
        pmm_last_word = i;

        return (uintptr_t)(total_bit_index * 4096);
    }

    for (size_t w = 0; w < pmm_last_word; w++) {
        if (pmm_bitmap[w] == 0xFFFFFFFFFFFFFFFFULL) {
            continue;
        }

        uint64_t inverted = ~pmm_bitmap[w];
        int bit_index = __builtin_ctzll(inverted); // returns first 0's index

        size_t total_bit_index = w * 64 + bit_index;
        
        pmm_bitmap[w] |= (1ULL << bit_index);
        pmm_last_word = w;

        return (uintptr_t)(total_bit_index * 4096);
    }

    return 0; // Out of memory :(
}

void pmm_free_page(uintptr_t physical_address) {
    if (physical_address % 4096 != 0) {
        return;
    }

    size_t page_index = physical_address / 4096;
    
    if (page_index >= pmm_total_pages) {
        return;
    }

    size_t word_index = page_index / 64;
    size_t bit_index = page_index % 64;

    pmm_bitmap[word_index] &= ~(1ULL << bit_index);

    if (word_index < pmm_last_word) {
        pmm_last_word = word_index;
    }
}


void pmm_init(void) {
    if(memmap_request.response == NULL || hhdm_request.response == NULL) {
        for (;;) { __asm__("hlt"); }
    }
    struct limine_memmap_response* responce = memmap_request.response;
    uint64_t hhdm_offset = hhdm_request.response->offset;

    uintptr_t highest_address = 0;

    for(uint64_t i = 0; i < responce->entry_count; i++) {
        struct limine_memmap_entry* entry = responce->entries[i];
        
        uintptr_t top_address = entry->base + entry->length;

        if(top_address % 4096 != 0) { // Aligning 
            top_address = (top_address + 4096) - (top_address % 4096);
        }
        
        if(top_address > highest_address) {
            highest_address = top_address;
        }
    }

    pmm_total_pages = highest_address / 4096;
    pmm_bitmap_size = pmm_total_pages / 64;
    if(pmm_total_pages % 64 != 0) pmm_bitmap_size++;

    pmm_bitmap_size_bytes = pmm_bitmap_size * 8;

    uintptr_t bitmap_physical_addr = 0;
    for(uint64_t i = 0; i < responce->entry_count; i++) {
        struct limine_memmap_entry* entry = responce->entries[i];

        if(entry->type == LIMINE_MEMMAP_USABLE && entry->length >= pmm_bitmap_size) {
            bitmap_physical_addr = entry->base;
            
            pmm_bitmap = (uint64_t*)(bitmap_physical_addr + hhdm_offset);
            break;
        }
    }
    if(pmm_bitmap == NULL) {
        for (;;) { __asm__("hlt"); }
    }

    for(size_t i = 0; i < pmm_bitmap_size; i++) {
        pmm_bitmap[i] = 0xFFFFFFFFFFFFFFFFULL;
    }

    for(uint64_t i = 0; i < responce->entry_count; i++) {
        struct limine_memmap_entry* entry = responce->entries[i];

        if(entry->type == LIMINE_MEMMAP_USABLE) {
            size_t start_page = entry->base / 4096;
            size_t page_count = entry->length / 4096;

            for(size_t p = 0; p < page_count; p++) {
                bitmap_clear(start_page + p);
            }
        }
    }

    // Since bitmap is in the USABLE memory region, we need to mark them as used because allocator functions may reserve them to other things
    size_t bitmap_start_page = bitmap_physical_addr / 4096;
    size_t bitmap_page_count = pmm_bitmap_size / 4096;
    if(pmm_bitmap_size % 4096 != 0) bitmap_page_count++;

    for(size_t p = 0; p < bitmap_page_count; p++) {
        bitmap_set(bitmap_start_page + p);
    }
    bitmap_set(0);
}