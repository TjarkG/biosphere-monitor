# Documentation for the Biosphere monitor command line tool

The Command line interface is started with `./biosphere`,
followed by a Port number (e.g. `/dev/ttyUSB0`) and optinal arguments,
which can ocure in any order, listed below:

* -h show this help message
* -r show current reading in Command line Output
* -s store all past readings in new file (stored as biosphere.csv in current directory)
* -f syncronize time
* -i set messurment intervall in s, use -i? to get current intervall
* -t run self test
* -g enable (1) or disable (0) Soil Sensor, -g? to show current state
* -ct set current ambient Temperature(°C), used for Offset Calibration

Commands to compile the Current Software after cloning:

* Command Line Tool: 'cc PC/biosphere.c -o PC/biosphere; ./PC/biosphere /dev/ttyUSB0 -r'
* Microcontroller: make program -C ./Microcontroller