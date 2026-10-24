CC = clang
LD = ld.lld
AS = nasm
XORRISO = xorriso

CFLAGS = --target=x86_64-elf -Wall -Wextra -std=c11 -O2 \
         -ffreestanding -fno-stack-protector -fno-stack-check \
         -fno-pic -fno-pie -fno-lto -m64 -march=x86-64 \
         -mno-80387 -mno-mmx -mno-sse -mno-sse2 -mno-red-zone \
         -mcmodel=kernel -Isrc

LDFLAGS = -m elf_x86_64 -T linker.ld --build-id=none
ASFLAGS = -f elf64

LIMINE_DIR = limine
ISO = OSewter.iso

.PHONY: all clean run

all: $(ISO)

src/kernel.o: src/kernel.c src/limine.h
	$(CC) $(CFLAGS) -c $< -o $@

OBJ = src/kernel.o \
      src/mm/pmm.o \
      src/drivers/serial.o \
      src/arch/x86_64/gdt/gdt.o \
      src/arch/x86_64/gdt/gdt_asm.o \
      src/arch/x86_64/idt/idt.o \
      src/arch/x86_64/idt/idt_asm.o

src/mm/pmm.o: src/mm/pmm.c src/mm/pmm.h src/limine.h
	$(CC) $(CFLAGS) -c $< -o $@

iso_root/boot/kernel.elf: $(OBJ) linker.ld
	mkdir -p $(@D)
	$(LD) $(LDFLAGS) -o $@ $(OBJ)

src/arch/x86_64/gdt/gdt.o: src/arch/x86_64/gdt/gdt.c src/arch/x86_64/gdt/gdt.h
	$(CC) $(CFLAGS) -c $< -o $@

src/arch/x86_64/gdt/gdt_asm.o: src/arch/x86_64/gdt/gdt.asm
	$(AS) $(ASFLAGS) $< -o $@

src/arch/x86_64/idt/idt.o: src/arch/x86_64/idt/idt.c src/arch/x86_64/idt/idt.h
	$(CC) $(CFLAGS) -c $< -o $@

src/arch/x86_64/idt/idt_asm.o: src/arch/x86_64/idt/idt.asm
	$(AS) $(ASFLAGS) $< -o $@

src/drivers/serial.o: src/drivers/serial.c src/drivers/serial.h
	$(CC) $(CFLAGS) -c $< -o $@

iso_root/limine.conf: src/limine.conf
	mkdir -p $(@D)
	cp $< $@

iso_root/boot/limine-bios.sys: $(LIMINE_DIR)/limine-bios.sys
	mkdir -p $(@D)
	cp $< $@

iso_root/boot/limine-bios-cd.bin: $(LIMINE_DIR)/limine-bios-cd.bin
	mkdir -p $(@D)
	cp $< $@

iso_root/boot/limine-uefi-cd.bin: $(LIMINE_DIR)/limine-uefi-cd.bin
	mkdir -p $(@D)
	cp $< $@

iso_root/EFI/BOOT/BOOTX64.EFI: $(LIMINE_DIR)/BOOTX64.EFI
	mkdir -p $(@D)
	cp $< $@


$(ISO): iso_root/boot/kernel.elf iso_root/limine.conf \
        iso_root/boot/limine-bios.sys iso_root/boot/limine-bios-cd.bin \
        iso_root/boot/limine-uefi-cd.bin iso_root/EFI/BOOT/BOOTX64.EFI
	$(XORRISO) -as mkisofs -b boot/limine-bios-cd.bin \
		-no-emul-boot -boot-load-size 4 -boot-info-table \
		--efi-boot boot/limine-uefi-cd.bin \
		-efi-boot-part --efi-boot-image --protective-msdos-label \
		iso_root -o $@
	./$(LIMINE_DIR)/limine bios-install $@

run: $(ISO)
	qemu-system-x86_64 -M q35 -m 2G -cdrom $(ISO) -serial stdio -no-reboot -no-shutdown

clean:
	rm -f src/kernel.o iso_root/boot/kernel.elf \
		iso_root/limine.conf \
		iso_root/boot/limine-bios.sys \
		iso_root/boot/limine-bios-cd.bin \
		iso_root/boot/limine-uefi-cd.bin \
		iso_root/EFI/BOOT/BOOTX64.EFI $(ISO)
