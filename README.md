# OSewter — 64-bit Long Mode Custom Operating System

Custom x86_64 operating system kernel built from scratch, targeting the **AMD64 Long Mode architecture**. The project is designed with a monolithic micro-framework hybrid structure, utilizing modern industry-standard toolchains and strict hardware isolation boundaries.

##  Architectural Architecture & Features

### Bootloader & Protocol Layer (`Limine v8.x`)
*   **Staged Execution:** Operates on top of the **Limine Boot Protocol** (Binary v8 Branch), avoiding legacy 16-bit real-mode constraints.
*   **Higher-Half Direct Map (HHDM):** Kernel executable sections are linked and mapped straight into the higher-half virtual address space (`0xffffffff80000000`).
*   **Linear Framebuffer:** Hardware-accelerated graphics initialization bypassing legacy VGA text modes.

### Memory Management Subsystem (`MM`)
*   **Physical Memory Manager (PMM):** High-performance **Bitmap Allocator** operating on 64-bit machine words (`uint64_t`).
*   **Hardware Acceleration:** Utilizes execution-level optimizations via compiler built-ins (`__builtin_ctzll`) executing the native `TZCNT`/`BSF` x86_64 instructions. Reduces search space time complexity significantly (O(N/64) word skips).
*   **Memory Map Processing:** Parses raw physical memory maps provided by the bootloader, ensuring safety zones for reserved ACPI, MMIO, and kernel data regions. Fully enforces physical `NULL` page insulation (`bitmap_set(0)`).

### Low-Level Segment & Interrupt Lifecycle (`Arch / x86_64`)
*   **Global Descriptor Table (GDT):** Flat Memory Model enforcement inside Ring 0 long mode. Implements explicit separate kernel/user code and data segment registers. Segment reloads achieved through strict 64-bit hardware Far Return mechanics (`REX.W + RETF`).
*   **Interrupt Descriptor Table (IDT):** Complete x86_64 16-byte gate descriptor layout covering the first 32 hardware CPU exceptions.
*   **Assembly Stub Alignment:** Low-level assembly wrapper (`idt.asm`) utilizing custom NASM macros to enforce context stack balance, capturing structural register frames (`r15` down to `rax`) alongside automatic error-code padding for uniform `System V AMD64 ABI` processing.
*   **Black-Box Diagnostics:** Custom low-level serial port driver (`drivers/serial.c`) pushing direct `outb` stream logs into the host `COM1` pipeline for bulletproof runtime `Kernel Panic` analytics.

---

##  Monolithic Toolchain Setup

The kernel utilizes an isolated **LLVM/Clang environment** inside MSYS2 to guarantee strict freestanding code compilation without native host operating system contamination.

*   **Compiler:** Clang (`--target=x86_64-elf`, freestanding, no-pic, no-pie, no-red-zone)
*   **Linker:** LLD (`ld.lld` enforcing `-m elf_x86_64` and structural object merging)
*   **Assembler:** NASM (`elf64` binary outputs)
*   **ISO Engine:** `xorriso` (Hybrid GPT/MBR embedding + Limine structural deployment via `bios-install`)

---

## Source Tree Layout

```text
OS/
├── limine/              # Precompiled bootloader deployment binaries (v8-binary)
├── src/                 # Main Freestanding Kernel Source
│   ├── arch/x86_64/     # Hardware Specific Architectures
│   │   ├── gdt/         # Segment Descriptors & Far Return Assemblies
│   │   └── idt/         # Exception Vectors & Assembly Stack Frames
│   ├── drivers/         # Bare-Metal Peripheral Drivers (Serial COM1)
│   ├── mm/              # High-Performance Memory Managers (PMM Word Bitmap)
│   ├── kernel.c         # Kernel Initialization Orchestrator & System Entry Point
│   └── limine.h         # Bootloader Protocol Header Definitions
│                        # Higher-Half ELF64 Memory Layout Linker Script
├── iso_root/            # Image Staging Root Directory
├── Makefile             # Clang64 Automations Script
└── linker.ld            # Higher-Half ELF64 Memory Layout Linker Script
```

---

## Building and Simulating

To compile the codebase, build the hybrid storage image, and fire up the system inside the QEMU emulator emulator, run the following commands inside the **MSYS2 CLANG64 Terminal**:

```bash
# Clean up cache and temporary object objects
make clean

# Compile everything, link the ELF, stamp the MBR, and boot up QEMU
make run
```

### Critical QEMU Emulation Parameters Injected:
*   `-M q35`: Emulates a modern Q35 chipset environment.
*   `-m 2G`: Restricts memory simulation footprint to 2 Gigabytes.
*   `-serial stdio`: Proxies the internal emulated `COM1` UART out to your host terminal window.
*   `-no-reboot -no-shutdown`: Completely freezes the CPU state on internal `hlt` commands during `Kernel Panic` cycles, preventing boot loops.
