#include <stddef.h>
#include <stdint.h>

void* memset(void* dest, int ch, size_t count) {
    uint8_t* byte_ptr = (uint8_t*)dest;
    uint8_t byte_val = (uint8_t)ch;

    while(count > 0 && ((uintptr_t)byte_ptr & 7) != 0) {
        *byte_ptr++ = byte_val;
        count--;
    }

    if(count >= 8) {
        // 64 bit word, where all of the 8 bytes equal to byte_val
        uint64_t word_val = ((uint64_t)byte_val << 56) |
                            ((uint64_t)byte_val << 48) |
                            ((uint64_t)byte_val << 40) |
                            ((uint64_t)byte_val << 32) |
                            ((uint64_t)byte_val << 24) |
                            ((uint64_t)byte_val << 16) |
                            ((uint64_t)byte_val << 8)  |
                            (uint64_t)byte_val;

        uint64_t* word_ptr = (uint64_t*)byte_ptr;
        size_t words = count / 8;
        
        for(size_t i = 0; i < words; i++) {
            *word_ptr++ = word_val;
        }

        byte_ptr = (uint8_t*)word_ptr;
        count %= 8;
    }

    while(count > 0) {
        *byte_ptr++ = byte_val;
        count--;
    }

    return dest;
}

void* memcpy(void* restrict dest, const void* restrict src, size_t count) {
    uint8_t* d = (uint8_t*)dest;
    const uint8_t* s = (const uint8_t*)src;

    if(((uintptr_t)d & 7) == ((uintptr_t)s & 7)) {
        while(count > 0 && ((uintptr_t)d & 7) != 0) {
            *d++ = *s++;
            count--;
        }
    }

    if(count >= 8) {
        uint64_t* word_d = (uint64_t*)d;
        const uint64_t* word_s = (const uint64_t*)s;
        size_t words = count / 8;

        for(size_t i = 0; i < words; i++) {
            *word_d++ = *word_s++;
        }

        d = (uint8_t*)word_d;
        s = (const uint8_t*)word_s;
        count %= 8;
    }

    while(count > 0) {
        *d++ = *s++;
        count--;
    }

    return dest;
}

void* memmove(void* dest, const void* src, size_t count) {
    uint8_t* d = (uint8_t*)dest;
    const uint8_t* s = (const uint8_t*)src;

    if(d == s || count == 0) {
        return dest;
    }

    if(d < s) {
        return memcpy(dest, src, count);
    }


    size_t bytes_to_copy = count % 8;
    while(bytes_to_copy > 0) {
        count--;
        d[count] = s[count];
        bytes_to_copy--;
    }

    if(count >= 8) {
        size_t words = count / 8;
        uint64_t* word_d = (uint64_t*)d;
        const uint64_t* word_s = (const uint64_t*)s;

        while(words > 0) {
            words--;
            word_d[words] = word_s[words];
        }
    }

    return dest;
}
