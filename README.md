# Getting started with the Biosphere monitor command line tool

Get the repository and all its dependencys by running
`/bin/bash -c "$(curl -fsSL  https://raw.githubusercontent.com/TjarkG/biosphere-monitor/main/Shell/install.sh)"`

The Command line interface is started with `./biosphere`,
followed by a Port number (e.g. `/dev/ttyUSB0`) and optinal arguments,
which can ocure in any order, listed below:

* -h    show this help message
* -r    show current reading in Command line Output
* -s    show all saved readings. use `./PC/biosphere /dev/ttyUSB0 -sc > biosphere.csv` to save them as a file
* -sc   same as -s, but with progress Updates on the Terminal. Usefull in combination with piping
* -f    syncronize time
* -i    set messurment intervall in s, use -i? to get current intervall
* -t    run self test
* -ct   set current ambient Temperature(°C), used for Offset Calibration
* -delete delete Flash

Commands to compile the Current Software after cloning:

* Command Line Tool: `cc PC/biosphere.c -o PC/biosphere; ./PC/biosphere /dev/ttyUSB0 -r`
* Microcontroller: `make program -C ./Microcontroller`

Complete Documentation (in German) can be found in /Documentation/Manual.pdf
