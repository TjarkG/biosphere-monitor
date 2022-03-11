cli: PC/cli.o biosphere.o tty.o
	gcc PC/cli.o PC/biosphere.o PC/tty.o -o biosphere

gui: gui.o biosphere.o guiArgs.aux tty.o glade
	gcc -o bioGui gui.o biosphere.o tty.o @Gui/guiArgs.aux
	rm Gui/guiArgs.aux

biosphere.o: PC/biosphere.c PC/tty.h PC/biosphere.h
	gcc -c PC/biosphere.c PC/tty.c

cli.o: PC/cli.c PC/biosphere.h PC/tty.h
	gcc -c cli.c

gui.o: Gui/gui.c Gui/signals.h guiArgs.aux
	gcc -c Gui/gui.c @Gui/guiArgs.aux

guiArgs.aux:
	pkg-config gtk+-3.0 --cflags --libs > Gui/guiArgs.aux
	echo "-rdynamic -Wall -Wextra" >> Gui/guiArgs.aux

glade: Gui/bioGui.glade
	cp Gui/bioGui.glade /usr/local/src

desktop: Gui/bioGui.desktop
	cp Gui/bioGui.desktop ~/.local/share/applications

%.o: %.c
	gcc -c $<

install: gui desktop
	sudo cp bioGui /usr/local/bin