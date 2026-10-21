#include "gdt.h"

static struct gdt_entry gdt[5];
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