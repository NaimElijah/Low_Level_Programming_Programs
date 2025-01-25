# format is target-name: target dependencies 
# {-tab-}actions

# All Targets
all: PartZeroExe

# Tool invocations
# Executable "PartZeroExe" depends on the files: main.o numbers.o and add.o.
PartZeroExe: main.o numbers.o add.o
	gcc -m32 -g -Wall -o PartZeroExe main.o numbers.o add.o

# Depends on the source and header files
main.o: main.c
	gcc -g -Wall -m32 -c -o main.o main.c

numbers.o: numbers.c
	gcc -g -Wall -m32 -c -o numbers.o numbers.c

add.o: add.s
	nasm -g -f elf -w+all -o add.o add.s

# Tell make that "clean" is not a file name!
.PHONY: clean

# Clean the build directory
clean:
	rm -f *.o hello
