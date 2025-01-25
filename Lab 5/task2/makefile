all: loader

loader : loader.o startup.o start.o
	ld -o loader loader.o startup.o start.o -L/usr/lib32 -lc -T linking_script -dynamic-linker /lib32/ld-linux.so.2

loader.o: loader.c 
	gcc -m32 -c loader.c -o loader.o

	

start: start.o
	 gcc -g -m32 -Wall -o start start.o

 # Depends on the source and header files 

start.o: start.s
	nasm -g -f elf -w+all -o start.o start.s  

startup: startup.o
	 gcc -g -m32 -Wall -o startup startup.o

 # Depends on the source and header files 

startup.o: startup.s
	nasm -g -f elf -w+all -o startup.o startup.s  

.PHONY: clean

clean:
	rm -f *.o loader