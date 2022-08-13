biosphere: cli.o biosphere.o tty.o
	gcc cli.o biosphere.o tty.o -o biosphere

runGui: bioGui
	./bioGui

bioGui: gui.o biosphere.o Gui/guiArgs.aux tty.o /usr/local/src/bioGui.glade
	gcc -o bioGui gui.o biosphere.o tty.o @Gui/guiArgs.aux

biosphere.o: PC/biosphere.c PC/tty.h PC/biosphere.h
	gcc -c PC/biosphere.c PC/tty.c

cli.o: PC/cli.c PC/biosphere.h PC/tty.h
	gcc -c PC/cli.c

gui.o: Gui/gui.c Gui/signals.h Gui/guiArgs.aux
	gcc -c Gui/gui.c @Gui/guiArgs.aux

Gui/guiArgs.aux:
	pkg-config gtk+-3.0 --cflags --libs > Gui/guiArgs.aux
	echo "-rdynamic -Wall -Wextra" >> Gui/guiArgs.aux

/usr/local/src/bioGui.glade: Gui/bioGui.glade
	sudo cp Gui/bioGui.glade /usr/local/src

~/.local/share/applications: Gui/bioGui.desktop
	cp Gui/bioGui.desktop ~/.local/share/applications

%.o: %.c
	gcc -c $<

install: gui ~/.local/share/applications
	sudo cp bioGui /usr/local/bin

all: 
	make -C Microcontroller all

program: 
	make -C Microcontroller program
