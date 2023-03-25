# Getting started with the Biosphere monitor command line tool

Get the repository and all its dependencies by running
`/bin/bash -c "$(curl -fsSL  https://raw.githubusercontent.com/TjarkG/biosphere-monitor/main/Shell/install.sh)"`

The Command line interface is started with `./biosphere`,
followed by a Port number (e.g. `/dev/ttyUSB0`) and optional arguments,
which can occur in any order, listed below:

* -h    show this help message
* -r    show current reading in Command line Output
* -rm   current output in machine-readable formate
* -s    show all saved readings. use pipes to save to a file
* -sc   same as -s, but with progress Updates on the Terminal. Use it to see the progress while piping
* -f    synchronize time
* -i    set measurement intervall in s, use -i? to get current intervall
* -t    run self test
* -ct   set current ambient Temperature(°C), used for Offset Calibration
* -delete delete Flash
* -gh   get current ADC reading of external input
* -ltn  set light on time, use -ltn? to get current setting
* -ltf  set light off time, use -ltf? to get current setting
* -lt   set light threshold, 0 for no threshold, use -lt? to get current setting

After cloning, generate makefiles with cmake:
```
mkdir cmake-build-debug
cd cmake-build-debug
cmake ..
```
and Compile the different programs:

* Command Line Tool: `make biosphere && ./biosphere /dev/ttyUSB0 -r`
* Graphical Configurator: `make bioGui`
* Microcontroller: `make Microcontroller.elf`

Check your data for Gaps with
`gawk -f Shell/gaps.awk gap=60 Documentation/Sample_Output.csv`
(gap is the intervall in s, each following argument is a Path to File to be checkt)

And convert it to an Excel Spreadsheet with
`soffice --convert-to xlsx --outdir ~/Dokumente/VS_Code/biosphere-monitor/Documentation Documentation/Sample_Output.csv --infilter=CSV:44,34,UTF8,true,3/10/4/10`

Complete Documentation (in German) can be found in /Documentation/Manual.pdf
