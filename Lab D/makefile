


all: multi


# Linking the object files to create the executable, linking with the C standard library
multi: multi.o
	ld -m elf_i386 -g -o multi multi.o -lc -dynamic-linker /lib/ld-linux.so.2

multi.o: multi.s
	nasm -f elf32 -g multi.s -o multi.o


.PHONY: clean

clean:
	rm -f *.o multi



