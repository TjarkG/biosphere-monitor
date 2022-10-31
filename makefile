biosphere: cli.o biosphere.o tty.o
	g++ cli.o biosphere.o tty.o -o biosphere

runGui: bioGui
	./bioGui

bioGui: gui.o PC/biosphere.c Gui/guiArgs.aux PC/tty.c /usr/local/src/bioGui.glade
	gcc -o bioGui gui.o PC/biosphere.c PC/tty.c @Gui/guiArgs.aux

biosphere.o: PC/biosphere.c PC/tty.h PC/biosphere.h
	g++ -c PC/biosphere.c

cli.o: PC/cli.cpp PC/biosphere.h PC/tty.h
	g++ -c PC/cli.cpp

tty.o: PC/tty.c PC/tty.h
	g++ -c PC/tty.c

gui.o: Gui/gui.c Gui/signals.h Gui/guiArgs.aux
	gcc -c Gui/gui.c @Gui/guiArgs.aux

Gui/guiArgs.aux:
	pkg-config gtk+-3.0 --cflags --libs > Gui/guiArgs.aux
	echo "-rdynamic -Wall -Wextra" >> Gui/guiArgs.aux

/usr/local/src/bioGui.glade: Gui/bioGui.glade
	sudo cp Gui/bioGui.glade /usr/local/src

~/.local/share/applications/bioGui.desktop: Gui/bioGui.desktop
	cp Gui/bioGui.desktop ~/.local/share/applications

install: bioGui ~/.local/share/applications/bioGui.desktop
	sudo cp bioGui /usr/local/bin

all: 
	make -C Microcontroller all

program: 
	make -C Microcontroller program
