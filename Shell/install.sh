#!/bin/bash
packagesNeeded='git gcc avr-libc binutils-avr gcc-avr avrdude make gawk libreoffice libreoffice-calc default-jre libreoffice-java-common mailutils'
if [ -x "$(command -v apk)" ];       then apk add --no-cache $packagesNeeded
elif [ -x "$(command -v apt-get)" ]; then sudo apt-get install $packagesNeeded
elif [ -x "$(command -v dnf)" ];     then sudo dnf install $packagesNeeded
elif [ -x "$(command -v zypper)" ];  then sudo zypper install $packagesNeeded
else echo "FAILED TO INSTALL PACKAGE: Package manager not found. You must manually install: $packagesNeeded">&2; fi
git clone https://github.com/TjarkG/biosphere-monitor
mkdir ~/BioData
cd biosphere-monitor
make -C PC cli
make all -C ./Microcontroller