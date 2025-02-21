

all: myshell mypipeline looper


myshell: LineParser.o myshell.o
	gcc -m32 -g -Wall -o myshell LineParser.o myshell.o

LineParser.o: LineParser.c
	gcc -g -Wall -m32 -c -o LineParser.o LineParser.c

myshell.o: myshell.c
	gcc -g -Wall -m32 -c -o myshell.o myshell.c


looper: looper.o
	gcc -m32 -g -Wall -o looper looper.o

looper.o: looper.c
	gcc -g -Wall -m32 -c -o looper.o looper.c


mypipeline: mypipeline.o
	gcc -m32 -g -Wall -o mypipeline mypipeline.o

mypipeline.o: mypipeline.c
	gcc -g -Wall -m32 -c -o mypipeline.o mypipeline.c






.PHONY: clean

clean:
	rm -f *.o myshell mypipeline looper
