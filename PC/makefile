cli: cli.o biosphere.o tty.o
	gcc cli.o biosphere.o tty.o -o biosphere

biosphere.o: biosphere.c tty.h biosphere.h
	gcc -c biosphere.c tty.c

cli.o: cli.c biosphere.h tty.h
	gcc -c cli.c

%.o: %.c
	gcc -c $<