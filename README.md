# Getting started with the Biosphere monitor command line tool

The Command line interface is started with `./biosphere`,
followed by a Port number (e.g. `/dev/ttyUSB0`) and optinal arguments,
which can ocure in any order, listed below:

* -h show this help message
* -r show current reading in Command line Output
* -s show all saved readings. use `./PC/biosphere /dev/ttyUSB0 -s > biosphere.csv` to save them as a file
* -f syncronize time
* -i set messurment intervall in s, use -i? to get current intervall
* -t run self test
* -ct set current ambient Temperature(°C), used for Offset Calibration
* -delete delete Flash

Commands to compile the Current Software after cloning:

* Command Line Tool: `cc PC/biosphere.c -o PC/biosphere; ./PC/biosphere /dev/ttyUSB0 -r`
* Microcontroller: `make program -C ./Microcontroller`
* Function Calculator: `cc Calculator/Calculator.c -o Calculator/calc -lm; ./Calculator/calc Calculator/in.csv`

Complete Documentation (in German) can be found in /Documentation/Manual.pdf