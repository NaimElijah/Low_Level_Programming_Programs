all: MenuMap                              #  all from the makefile example in the moodle you gave, I know how to do this(from spl course) and adjusted it accordingly.

MenuMap: menu_map.o                              #  from the makefile example in the moodle
	gcc -m32 -g -Wall -o MenuMap menu_map.o

menu_map.o: menu_map.c                              #  from the makefile example in the moodle
	gcc -g -Wall -m32 -c -o menu_map.o menu_map.c

.PHONY: clean                              #  from the makefile example in the moodle

clean:                              #  from the makefile example in the moodle
	rm -f *.o MenuMap
