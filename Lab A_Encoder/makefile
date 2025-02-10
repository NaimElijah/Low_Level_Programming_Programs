
all: encoder    #  this file's code is from the makefile examples, I already know how to do this from spl course so I adjusted it accordingly

encoder: encoder.o      #  my line
	gcc -m32 -g -Wall -o encoder encoder.o

encoder.o: encoder.c      #  my line
	gcc -g -Wall -m32 -c -o encoder.o encoder.c

.PHONY: clean      #  my line

clean:      #  my line
	rm -f *.o encoder
