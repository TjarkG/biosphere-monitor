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

	#comment next paragraph out when transmitting already saved data
	if(system(biopath " " $1 " -s >" filepath) != 0)
	{
		message = "An Error occured saving Data"
	}

	#convert csv to xlsx
	system("soffice --convert-to xlsx --outdir ~/BioData " filepath " --infilter=CSV:44,34,UTF8,true,3/10/4/10 > /dev/null")

	cmd = "gawk -f " gappath " gap=600 " filepath
    cmd | getline gaps
	message = message "\n"

	system("echo \"Hallo \"" $2 "\",\n" message "im Anhang findest du die neusten Messwerte von deiner Biosphaere.\nMfG, AstroBot\n\n(Diese Nachricht wurde automatisch versendet)\" | mutt -s \"" heading "\" " $3 " -a " filepath " -a " expath)

	if(oldPort == $1)
		print "Warning: Multiple Entrys for " $1

	oldPort = $1
}
