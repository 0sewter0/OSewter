#include "idt.h"
#include "drivers/serial.h"

extern void* isr_stub_table[];

// A table of 256 interrupt descriptors
static struct idt_entry idt[256];
static struct idt_pointer idt_ptr;

// Internal function for registering an interrupt handler
void idt_set_descriptor(uint8_t vector, void* isr, uint8_t attributes) {
    uint64_t addr = (uint64_t)isr;

    idt[vector].isr_low    = (uint16_t)(addr & 0xFFFF);
    idt[vector].kernel_cs  = 0x08; // kernel code segment
    idt[vector].ist        = 0;
    idt[vector].attributes = attributes;
    idt[vector].isr_mid    = (uint16_t)((addr >> 16) & 0xFFFF);
    idt[vector].isr_high   = (uint32_t)((addr >> 32) & 0xFFFFFFFF);
    idt[vector].reserved   = 0;
}

void idt_init(void) {
    idt_ptr.limit = (sizeof(struct idt_entry) * 256) - 1;
    idt_ptr.base  = (uint64_t)&idt;

    // The attribute flag 0x8E means:
    // Bit 7 (Present) = 1 (Valid descriptor)
    // Bits 6-5 (DPL)  = 00 (Ring 0)
    // Bit 4 (Storage) = 0 (For interrupt gates)
    // Bits 3-0 (Type) = 1110 (0xE - 64-bit Interrupt Gate)

     for (uint8_t vector = 0; vector < 32; vector++) {
        idt_set_descriptor(vector, isr_stub_table[vector], 0x8E);
    }
    
    __asm__ volatile ("lidt %0" :: "m"(idt_ptr));
    
    __asm__ volatile ("sti");
}

static const char* exception_messages[] = {
    "Division By Zero", "Debug", "Non Maskable Interrupt", "Breakpoint",
    "Into Detected Overflow", "Out of Bounds", "Invalid Opcode", "No Coprocessor",
    "Double Fault", "Coprocessor Segment Overrun", "Bad TSS", "Segment Not Present",
    "Stack Fault", "General Protection Fault", "Page Fault", "Unknown Interrupt",
    "Fault", "Machine Check", "SIMD Floating-Point", "Virtualization",
    "Control Protection", "Reserved", "Reserved", "Reserved",
    "Reserved", "Reserved", "Reserved", "Reserved",
    "Reserved", "Reserved", "Hypervisor Injection", "VMM Communication"
};

void idt_handler(struct interrupt_frame* frame) {
    if(frame->interrupt_number < 32) {
        serial_print("\n  KERNEL PANIC: Unhandled Processor Exception\n");

        serial_print("Exception: ");
        if(frame->interrupt_number < 16) {
            serial_print(exception_messages[frame->interrupt_number]);
        } else {
            serial_print("Reserved/Unknown Fault");
        }
        serial_print("\n");
        // if Page Fault (14), reading the register CR2.
        // It is in CR2 that the processor records the exact memory address that the program has touched
        if(frame->interrupt_number == 14) {
            uint64_t faulty_address;
            __asm__ volatile("mov %%cr2, %0" : "=r"(faulty_address));

            serial_print("CR2 (Faulty Memory Address): "); // Cannot write to output HEX number yet
            serial_print("Address captured into CR2 successfully.\n");
        }

        serial_print("Crash location (RIP): Code execution halted.\n");

        for (;;) { __asm__("hlt"); }
    }
}
