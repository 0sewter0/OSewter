### Roadmap

# Phase 1
**Goal: Allocate and release physical memory**

[ ] Parse the memory map supplied by Limine
[ ] Reserve memery occupied by the kernel and boot information
[ ] Implement a physical page allocator
[ ] Write a bitmap allocator
[ ] Test allocation boundaries and memory exhaustion

# Phase 2
**Goal: Handle CPU faults instead of silently crashing**

[ ] Write assembly interrupt and exception stubs
[ ] Implement handlers for major CPU exceptions
[ ] Print register state when an exception occurs
[ ] Handle page faults with diagnostic information
[ ] Configure interrupt controllers and IRQ handling