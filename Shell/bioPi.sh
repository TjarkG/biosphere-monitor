#!/bin/bash
#started with:
#bash ~/biosphere-monitor/Shell/bioPi.sh
awk '
BEGIN {
	FS = "\" \""
	"date +%Y-%m-%d" | getline date;
	heading="Biosphaeren Daten";
	system("cc ~/biosphere-monitor/PC/biosphere.c -o ~/biosphere-monitor/PC/biosphere")
	#system("mkdir ~/BioData")
}
# skip comments starting with #
$0 ~ /^#/ {next}
{
	$1 = substr($1, 2, length($1))
	$3 = substr($3, 1, length($3)-1)
	nameFile=$2
	gsub(/\ /,"-" , nameFile)

	filepath=sprintf("~/BioData/biosphere-%s-%s.csv", nameFile, date)

	if(system("~/biosphere-monitor/PC/biosphere " $1 " -s >" filepath) != 0)
		message="An Error occured saving Data"

	system("echo \"Hallo \"" $2 "\",\n" message \
	"im Anhang findest du die neusten Messwerte von deiner Biosphaere.\n\
	MfG, AstroBot\n\n\
	(Diese Nachricht wurde automatisch versendet)\" | mail -s \"" \
	heading "\" " $3 " -A " filepath)
}
' ~/Ports.config
