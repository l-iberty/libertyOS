.PHONY: all img clean

ASMINCLUDE=	-Iinclude/
ASMFMT=		-f elf
LDFLAGS=	-s -melf_i386

all: boot.bin loader.bin kernel.bin

boot.bin: boot.asm
	nasm $(ASMINCLUDE) $< -o $@

loader.bin: loader.asm
	nasm $(ASMINCLUDE) $< -o $@

kernel.bin: kernel.o
	ld $(LDFLAGS) $< -o $@

kernel.o: kernel.asm
	nasm $(ASMINCLUDE) $(ASMFMT) $< -o $@

img:
	dd if=boot.bin of=a.img bs=512 count=1 conv=notrunc
	sudo mount -o loop a.img /mnt/floppy
	sudo cp loader.bin /mnt/floppy
	sudo cp kernel.bin /mnt/floppy
	sudo umount /mnt/floppy

clean:
	rm *.o *.bin
