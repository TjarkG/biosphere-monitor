Linux: biosphere.o
	gcc biosphere.o -o biosphere;

biosphere.o: biosphere.c tty.h
	gcc -c biosphere.c

%.o: %.c
	gcc -c $<