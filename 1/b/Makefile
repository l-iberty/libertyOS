.PHONY: all

all:
	nasm boot.asm -o boot.bin
	nasm loader.asm -o loader.bin
	dd if=boot.bin of=a.img bs=512 count=1 conv=notrunc
	sudo mount -o loop a.img /mnt/floppy
	sudo cp loader.bin /mnt/floppy
	sudo umount /mnt/floppy
