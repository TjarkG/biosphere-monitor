#!/bin/bash
#setup
echo "Drücke irgendeine Taste zum beenden"
path=$( cd "$(dirname "${BASH_SOURCE[0]}")" || exit ; cd ..; pwd -P )
function cleanup() {
    tput cnorm
}

trap cleanup EXIT
"$path/cmake-build-debug/biosphere" "$1" -f
tput civis
#loop
while [[ -z "$IN" ]]; do
"$path/cmake-build-debug/biosphere" "$1" -rm |
awk -F, \
'{print "\033cAktuelle Messwerte:\n" \
$1 " UTC\n" $2 " lux\tHelligkeit\n" \
$3 "°C \tRaumtemperatur\n\n\
In der Biosphäre:\n" \
$4 "°C\n" \
$5 "hPa\n" \
$6 "% \tLuftfeuchtigkeit\n" \
$7 "% \tBodenfeuchtigkeit"}'
read  -r -t 1 -N 1 IN
done
tput cnorm
