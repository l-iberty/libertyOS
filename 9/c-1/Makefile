.PHONY: all image clean

ASMBFLAGS=	-I boot/include/
ASMKFLAGS=	-I include/ -f elf
LDKFLAGS=	-melf_i386 -Ttext 0x1b400 -Map kernel.map -s # -s 剔除符号信息
CFLAGS=		-I include/ -c -m32 -fno-stack-protector -fno-builtin -std=c99 # -g 加入符号信息

KRNLOBJS=	kernel/kernel.o kernel/start.o kernel/8259A.o kernel/main.o \
		kernel/protect.o kernel/clock.o kernel/syscall.o kernel/proc.o \
		kernel/irq.o kernel/exception.o kernel/test.o

DRVOBJS=	driver/keyboard.o driver/tty.o driver/hd.o driver/console.o

LIBOBJS=	lib/klib.o lib/string.o lib/printf.o lib/vsprintf.o lib/io.o

FSOBJS=		fs/main.o fs/open.o fs/misc.o fs/close.o fs/rdwt.o fs/unlink.o

MMOBJS=		mm/main.o mm/vmalloc.o mm/vmfree.o mm/misc.o mm/brk.o mm/malloc/malloc.o

EXEOBJS=	exe/main.o exe/fork.o

KOBJS=		$(KRNLOBJS) $(DRVOBJS) $(LIBOBJS) $(FSOBJS) $(MMOBJS) $(EXEOBJS)

all: boot.bin loader.bin kernel.bin

####################################################################

boot.bin: boot/boot.asm
	nasm $(ASMBFLAGS) $< -o $@

loader.bin: boot/loader.asm
	nasm $(ASMBFLAGS) $< -o $@

kernel.bin: $(KOBJS)
	ld $(LDKFLAGS) $^ -o $@

####################################################################

kernel/kernel.o: kernel/kernel.asm
	nasm -I kernel/ $(ASMKFLAGS) $< -o $@

kernel/start.o: kernel/start.c
	gcc $(CFLAGS) $< -o $@

kernel/8259A.o: kernel/8259A.asm
	nasm $(ASMKFLAGS) $< -o $@

kernel/main.o: kernel/main.c
	gcc $(CFLAGS) $< -o $@

kernel/protect.o: kernel/protect.c
	gcc $(CFLAGS) $< -o $@

kernel/clock.o: kernel/clock.c
	gcc $(CFLAGS) $< -o $@

kernel/syscall.o: kernel/syscall.asm
	nasm $(ASMKFLAGS) $< -o $@

kernel/proc.o: kernel/proc.c
	gcc $(CFLAGS) $< -o $@

kernel/irq.o: kernel/irq.c
	gcc $(CFLAGS) $< -o $@

kernel/exception.o: kernel/exception.c
	gcc $(CFLAGS) $< -o $@
	
kernel/test.o: kernel/test.c
	gcc $(CFLAGS) $< -o $@
	
driver/keyboard.o: driver/keyboard.c
	gcc $(CFLAGS) $< -o $@

driver/tty.o: driver/tty.c
	gcc $(CFLAGS) $< -o $@
	
driver/hd.o: driver/hd.c
	gcc $(CFLAGS) $< -o $@

driver/console.o: driver/console.c
	gcc $(CFLAGS) $< -o $@

lib/klib.o: lib/klib.asm
	nasm $(ASMKFLAGS) $< -o $@

lib/string.o: lib/string.c
	gcc $(CFLAGS) $< -o $@

lib/printf.o: lib/printf.c
	gcc $(CFLAGS) $< -o $@

lib/vsprintf.o: lib/vsprintf.c
	gcc $(CFLAGS) $< -o $@

lib/io.o: lib/io.c
	gcc $(CFLAGS) $< -o $@

fs/main.o: fs/main.c
	gcc $(CFLAGS) $< -o $@

fs/open.o: fs/open.c
	gcc $(CFLAGS) $< -o $@

fs/misc.o: fs/misc.c
	gcc $(CFLAGS) $< -o $@

fs/close.o: fs/close.c
	gcc $(CFLAGS) $< -o $@

fs/rdwt.o: fs/rdwt.c
	gcc $(CFLAGS) $< -o $@

fs/unlink.o: fs/unlink.c
	gcc $(CFLAGS) $< -o $@

mm/main.o: mm/main.c
	gcc $(CFLAGS) $< -o $@

mm/vmalloc.o: mm/vmalloc.c
	gcc $(CFLAGS) $< -o $@

mm/vmfree.o: mm/vmfree.c
	gcc $(CFLAGS) $< -o $@

mm/misc.o: mm/misc.c
	gcc $(CFLAGS) $< -o $@

mm/brk.o: mm/brk.c
	gcc $(CFLAGS) $< -o $@

mm/malloc/malloc.o: mm/malloc/malloc.c
	gcc $(CFLAGS) $< -o $@

exe/main.o: exe/main.c
	gcc $(CFLAGS) $< -o $@

exe/fork.o: exe/fork.c
	gcc $(CFLAGS) $< -o $@

####################################################################

image:
	dd if=boot.bin of=a.img bs=512 count=1 conv=notrunc
	sudo mount -o loop a.img /mnt/floppy
	sudo cp loader.bin /mnt/floppy
	sudo cp kernel.bin /mnt/floppy
	sudo umount -l /mnt/floppy

####################################################################

clean:
	rm `find . -name "*.o"`
	rm `find . -name "*.bin"`	
	rm *.txt
