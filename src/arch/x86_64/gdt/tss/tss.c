#include <stdint.h>
#include "tss.h"
#include "arch/x86_64/gdt/gdt.h"
#include "klib/string.h"

__attribute__((aligned(16))) static uint8_t double_fault_stack[4096];

void init_tss() {
    memset(&tss, 0, sizeof(tss));

    tss.ist[0] = (uint64_t)&double_fault_stack[sizeof(double_fault_stack)];

    tss.iomap_base = sizeof(tss);

    add_tss();

    __asm__ volatile("ltr %%ax" :: "a"(0x28) : "memory"); // Turning on the TSS in the processor. Selector: 0x28
}

