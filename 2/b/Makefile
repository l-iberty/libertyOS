.PHONY: clean

LDFLAGS=	-melf_i386 -s
ASMFLAGS=	-f elf
GCCFLAGS=	-c -m32

foobar: foo.o bar.o
	ld $(LDFLAGS) $^ -o $@

foo.o: foo.asm
	nasm $(ASMFLAGS) $< -o $@

bar.o: bar.c
	gcc $(GCCFLAGS) $< -o $@

clean:
	rm *.o
