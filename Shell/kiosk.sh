#!/bin/bash
#setup
cc ../PC/biosphere.c -o ../PC/biosphere
./../PC/biosphere /dev/ttyUSB0 -f
#loop
./../PC/biosphere /dev/ttyUSB0 -rm
crMess='10.12.2021 15:31:00,2,23.2,23.6,947,26,10,0'
: 'yad \
--title="Biosphere Display Mode" \
--text-align=center \
--text="<span font='64'>Aktuelle Messwerte:\n""${crMess}""</span>" \
--no-buttons \
--width=3840 \
--height=2160'