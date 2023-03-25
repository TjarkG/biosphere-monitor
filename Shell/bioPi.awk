#!/bin/awk
#started with:
# awk -f ~/biosphere-monitor/Shell/bioPi.awk ~/Ports.config
BEGIN {
	FS = "\" \""
	"date +%Y-%m-%d" | getline date;
	Heading = "Biosphaeren Daten";
	OldPort = "";
}
# skip comments starting with #
$0 !~ /^#/ {
	$1 = substr($1, 2, length($1))
	$3 = substr($3, 1, length($3)-1)
	nameFile = $2
	gsub(/ /,"-" , nameFile)

	filepath = sprintf("~/BioData/biosphere-%s-%s.csv", nameFile, date)
	expath 	 = sprintf("~/BioData/biosphere-%s-%s.xlsx", nameFile, date)

	#comment next paragraph out when transmitting already saved data
	if(system("cmake-build-debug/biosphere" " " $1 " -s >" filepath) != 0)
	{
		message = "An Error occured saving Data"
	}

	cmd = "gawk -f Shell/gaps.awk gap=600 " filepath
    cmd | getline gaps

	system("echo \"Hallo \"" $2 "\",\n" message "Im Anhang findest du die neusten Messwerte von deiner Biosphaere.\nMfG, AstroBot\n\n(Diese Nachricht wurde automatisch versendet)\" | mutt -s \"" Heading "\" " $3 " -a " filepath)

	if(OldPort == $1)
		print "Warning: Multiple Entrys for " $1

	OldPort = $1
}
