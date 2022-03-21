#!/bin/awk
#started with:
# awk -f ~/biosphere-monitor/Shell/bioPi.awk ~/Ports.config
BEGIN {
	FS = "\" \""
	"date +%Y-%m-%d" | getline date;
	heading = "Biosphaeren Daten";
	oldPort = "";
	"locate biosphere-monitor/Shell/gaps.awk -l 1" | getline gappath;
	"locate biosphere-monitor/PC/biosphere -l 1" | getline biopath;
}
# skip comments starting with #
$0 !~ /^#/ {
	$1 = substr($1, 2, length($1))
	$3 = substr($3, 1, length($3)-1)
	nameFile = $2
	gsub(/ /,"-" , nameFile)

	filepath = sprintf("~/BioData/biosphere-%s-%s.csv", nameFile, date)
	expath 	 = sprintf("~/BioData/biosphere-%s-%s.xlsx", nameFile, date)

	if(system(biopath " " $1 " -s >" filepath) != 0)
	{
		message = "An Error occured saving Data"
	}

	cmd = "gawk -f " gappath " gap=600 " filepath
    cmd | getline gaps
	message = message "\n" gaps "\n"

	system("echo \"Hallo \"" $2 "\",\n" message \
	"im Anhang findest du die neusten Messwerte von deiner Biosphaere.\n\
	MfG, AstroBot\n\n\
	(Diese Nachricht wurde automatisch versendet)\" | mail -s \"" \
	heading "\" " $3 " -A " filepath )

	if(oldPort == $1)
		print "Warning: Multiple Entrys for " $1

	oldPort = $1
}
