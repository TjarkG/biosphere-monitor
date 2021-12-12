#!/bin/bash
packagesNeeded='git gcc avr-libc binutils-avr gcc-avr avrdude make'
if [ -x "$(command -v apk)" ];       then apk add --no-cache $packagesNeeded
elif [ -x "$(command -v apt-get)" ]; then sudo apt-get install $packagesNeeded
elif [ -x "$(command -v dnf)" ];     then sudo dnf install $packagesNeeded
elif [ -x "$(command -v zypper)" ];  then sudo zypper install $packagesNeeded
else echo "FAILED TO INSTALL PACKAGE: Package manager not found. You must manually install: $packagesNeeded">&2; fi
git clone https://github.com/TjarkG/biosphere-monitor
cd biosphere-monitor
cc PC/biosphere.c -o PC/biosphere;
make all -C ./Microcontroller