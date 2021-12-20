#!/bin/bash
#setup
echo "Drücke irgendeine Taste zum beenden"
path=$( cd "$(dirname "${BASH_SOURCE[0]}")" ; cd ..; pwd -P )
echo $path
function cleanup() {
    tput cnorm
}

trap cleanup EXIT
cc "$path/PC/biosphere.c" -o "$path/PC/biosphere"
"$path/PC/biosphere" $1 -f
tput civis
sleep 1
#loop
while [[ -z "$IN" ]]; do
"$path/PC/biosphere" $1 -rm |
awk -F, \
'{print "\033cAktuelle Messwerte:\n" \
$1 " UTC\n" $2 " lux\tHelligkeit\n" \
$3 "°C \tRaumtemperatur\n\n\
In der Biosphpäre:\n" \
$4 "°C\n" \
$5 "hPa\n" \
$6 "% \tLuftfeuchtigkeit\n" \
$7 "% \tBodenfeuchtigkeit"}'
read -t 1 -N 1 IN
done
tput cnorm
