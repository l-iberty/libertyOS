.PHONY: all image clean

ASMBFLAGS=	-I boot/include/
ASMKFLAGS=	-I include/ -f elf
LDKFLAGS=	-s -melf_i386 -Ttext 0x1b400 -Map kernel.map # -s 剔除符号信息
CFLAGS=		-I include/ -c -m32 -fno-stack-protector -fno-builtin # -g3 加入符号信息

KOBJS=		kernel.o start.o 8259A.o main.o protect.o clock.o syscall.o \
		proc.o irq.o keyboard.o tty.o hd.o console.o klib.o string.o \
		printf.o vsprintf.o io.o fs_main.o open.o misc.o close.o \
		rdwt.o unlink.o mm_main.o vmalloc.o vmfree.o exe_main.o fork.o

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

start.o: kernel/start.c
	gcc $(CFLAGS) $< -o $@

8259A.o: kernel/8259A.asm
	nasm $(ASMKFLAGS) $< -o $@

main.o: kernel/main.c
	gcc $(CFLAGS) $< -o $@

protect.o: kernel/protect.c
	gcc $(CFLAGS) $< -o $@

clock.o: kernel/clock.c
	gcc $(CFLAGS) $< -o $@

syscall.o: kernel/syscall.asm
	nasm $(ASMKFLAGS) $< -o $@

proc.o: kernel/proc.c
	gcc $(CFLAGS) $< -o $@

irq.o: kernel/irq.c
	gcc $(CFLAGS) $< -o $@
	
keyboard.o: driver/keyboard.c
	gcc $(CFLAGS) $< -o $@

tty.o: driver/tty.c
	gcc $(CFLAGS) $< -o $@
	
hd.o: driver/hd.c
	gcc $(CFLAGS) $< -o $@

console.o: driver/console.c
	gcc $(CFLAGS) $< -o $@

klib.o: lib/klib.asm
	nasm $(ASMKFLAGS) $< -o $@

string.o: lib/string.asm
	nasm $(ASMKFLAGS) $< -o $@

printf.o: lib/printf.c
	gcc $(CFLAGS) $< -o $@

vsprintf.o: lib/vsprintf.c
	gcc $(CFLAGS) $< -o $@

io.o: lib/io.c
	gcc $(CFLAGS) $< -o $@

fs_main.o: fs/fs_main.c
	gcc $(CFLAGS) $< -o $@

open.o: fs/open.c
	gcc $(CFLAGS) $< -o $@

misc.o: fs/misc.c
	gcc $(CFLAGS) $< -o $@

close.o: fs/close.c
	gcc $(CFLAGS) $< -o $@

rdwt.o: fs/rdwt.c
	gcc $(CFLAGS) $< -o $@

unlink.o: fs/unlink.c
	gcc $(CFLAGS) $< -o $@

mm_main.o: mm/mm_main.c
	gcc $(CFLAGS) $< -o $@

vmalloc.o: mm/vmalloc.c
	gcc $(CFLAGS) $< -o $@

vmfree.o: mm/vmfree.c
	gcc $(CFLAGS) $< -o $@

exe_main.o: exe/exe_main.c
	gcc $(CFLAGS) $< -o $@

fork.o: exe/fork.c
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
