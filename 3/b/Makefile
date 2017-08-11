.PHONY: all image clean

ASMBFLAGS=	-I boot/include/
ASMKFLAGS=	-f elf
LDKFLAGS=	-s -melf_i386 -Ttext 0x1b200
CFLAGS=		-I include/ -c -m32 -fno-stack-protector

KOBJS=		kernel.o klib.o start.o 8259A.o main.o protect.o

all: boot.bin loader.bin kernel.bin

####################################################################

boot.bin: boot/boot.asm
	nasm $(ASMBFLAGS) $< -o $@

loader.bin: boot/loader.asm
	nasm $(ASMBFLAGS) $< -o $@

kernel.bin: $(KOBJS)
	ld $(LDKFLAGS) $^ -o $@

####################################################################

kernel.o: kernel/kernel.asm
	nasm -I kernel/ $(ASMKFLAGS) $< -o $@

klib.o: lib/klib.asm
	nasm $(ASMKFLAGS) $< -o $@

start.o: kernel/start.c
	gcc $(CFLAGS) $< -o $@

8259A.o: kernel/8259A.asm
	nasm $(ASMKFLAGS) $< -o $@

main.o: kernel/main.c
	gcc $(CFLAGS) $< -o $@

protect.o: kernel/protect.c
	gcc $(CFLAGS) $< -o $@

####################################################################

image:
	dd if=boot.bin of=a.img bs=512 count=1 conv=notrunc
	sudo mount -o loop a.img /mnt/floppy
	sudo cp loader.bin /mnt/floppy
	sudo cp kernel.bin /mnt/floppy
	sudo umount /mnt/floppy

####################################################################

clean:
	rm *.o *.bin
