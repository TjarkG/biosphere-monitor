#!/bin/bash
sudo apt-get install git gcc avr-libc binutils-avr gcc-avr avrdude make
git clone https://github.com/TjarkG/biosphere-monitor
cd biosphere-monitor
cc PC/biosphere.c -o PC/biosphere;
make all -C ./Microcontroller