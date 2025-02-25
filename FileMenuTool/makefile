
all: task1 task4


task1: task1.o
	gcc -m32 task1.o -o task1

task4: task4.o
	gcc -m32 -fno-pie -fno-stack-protector task4.o -o task4



task1.o: task1.c
	gcc -m32 -Wall -c task1.c -o task1.o

task4.o: task4.c
	gcc -m32 -fno-pie -fno-stack-protector -Wall -c task4.c -o task4.o

.PHONY: clean

clean:
	rm -f *.o task1 task4


