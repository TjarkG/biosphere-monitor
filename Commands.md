# Commands for the Biosphere monitor command line tool

The Command line interface is started with `run ./biosphere`,
followed by a Port number (e.g. `/dev/ttyUSB0`) and optinal arguments,
which can ocure in any order, which are listed below:
* -r show current reading in Command line Output
* -s store all past readings in new file (stored as Data.csv in current directory)
* -t force time syncronization, even with big deviations
* -i set messurment intervall in s, use -i? to get current intervall
* -t run self test
* -g enable (1) or disable (0) Soil Sensor, ? to show current state