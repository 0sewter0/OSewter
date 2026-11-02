#include "gdt.h"
#include "tss/tss.h"

static struct gdt_entry gdt[7];
static struct gdt_ptr gdt_pointer;

extern void gdt_load(struct gdt_ptr* ptr);

static void gdt_set_gate(int num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran) {
    gdt[num].base_low    = (base & 0xFFFF);
    gdt[num].base_middle = (base >> 16) & 0xFF;
    gdt[num].base_high   = (base >> 24) & 0xFF;

    gdt[num].limit_low   = (limit & 0xFFFF);
    gdt[num].granularity = (limit >> 16) & 0x0F;

    gdt[num].granularity |= gran & 0xF0;
    gdt[num].access      = access;
}

void gdt_set_tss(int num, uint64_t tss_addr, uint32_t limit) {
    struct gdt_entry* low_slot  = &gdt[num];

    // 0x89 (Present, Executable/TSS, Descriptor Privilege Level = 0)
    low_slot->limit_low   = (limit & 0xFFFF);
    low_slot->granularity = (limit >> 16) & 0x0F;
    low_slot->granularity |= 0x00;
    
    low_slot->access      = 0x89;  // 0b10001001 

    low_slot->base_low    = (tss_addr & 0xFFFF);
    low_slot->base_middle = (tss_addr >> 16) & 0xFF;
    low_slot->base_high   = (tss_addr >> 24) & 0xFF;

    uint32_t* high_slot_raw = (uint32_t*)&gdt[num + 1];
    
    high_slot_raw[0] = (uint32_t)(tss_addr >> 32); 
    high_slot_raw[1] = 0;
}

void gdt_init(void) {
    gdt_pointer.limit = (sizeof(struct gdt_entry) * 5) - 1;
    gdt_pointer.base = (uint64_t)&gdt;

    // 0x00 - Null Descriptor
    gdt_set_gate(0, 0, 0, 0, 0);

    // 0x08 - Kernel Code. Access 0x9A (Present, Ring 0, Executable, Read), Granularity 0x20 (Long Mode Flag 'L')
    gdt_set_gate(1, 0, 0, 0x9A, 0x20);

    // 0x10 - Kernel Data. Access 0x92 (Present, Ring 0, Writable)
    gdt_set_gate(2, 0, 0, 0x92, 0x00);

    // 0x18 - User Data. Access 0xF2 (Present, Ring 3, Writable)
    gdt_set_gate(3, 0, 0, 0xF2, 0x00);

    // 0x20 - User Code. Access 0xFA (Present, Ring 3, Executable, Read), Granularity 0x20 (Long Mode)
    gdt_set_gate(4, 0, 0, 0xFA, 0x20);

    gdt_load(&gdt_pointer);
}

void add_tss() {
    uint64_t tss_base = (uint64_t)&tss;
    uint32_t tss_limit = sizeof(tss) - 1;
    gdt_set_tss(5, tss_base, tss_limit);

    gdt_load(&gdt_pointer);
}