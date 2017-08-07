.PHONY:

LDFLAGS=	-melf_i386 -s
ASMFLAGS=	-f elf

hello: hello.o
	ld $(LDFLAGS) $< -o $@

hello.o: hello.asm
	nasm $(ASMFLAGS) $< -o $@
