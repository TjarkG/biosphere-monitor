#!/bin/bash
#setup
echo "Drücke irgendeine Taste zum beenden"
cc ../PC/biosphere.c -o ../PC/biosphere
./../PC/biosphere /dev/ttyUSB0 -f
tput civis
sleep 2
#loop
while [[ -z "$IN" ]]; do
./../PC/biosphere /dev/ttyUSB0 -rm |
awk -F, \
'{print "\033cAktuelle Messwerte:\n" \
$1 " UTC\n" $2 " lux \tHelligkeit\n" \
$3 "°C \tRaumtemperatur\n\n\
In der Biosphpäre:\n" \
$4 "°C\n" \
$5 "hPa\n" \
$6 "% \tLuftfeuchtigkeit\n" \
$7 "% \tBodenfeuchtigkeit"}'
read -t 1 -N 1 IN
done
tput cnorm
