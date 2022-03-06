cli: cli.o biosphere.o tty.o
	gcc cli.o biosphere.o tty.o -o biosphere

gui: gui.o guiArgs.aux
	gcc -o bioGui gui.o @guiArgs.aux
	rm guiArgs.aux

biosphere.o: biosphere.c tty.h biosphere.h
	gcc -c biosphere.c tty.c

cli.o: cli.c biosphere.h tty.h
	gcc -c cli.c

gui.o: gui.c signals.h guiArgs.aux
	gcc -c gui.c @guiArgs.aux

guiArgs.aux:
	pkg-config gtk+-3.0 --cflags --libs > guiArgs.aux
	echo "-rdynamic -Wall -Wextra" >> guiArgs.aux

%.o: %.c
	gcc -c $<