#/bin/bash
awk '
BEGIN {
	"date +%Y-%m-%d" | getline date;
	heading="Biosphaeren Daten";
	system("cc ../PC/biosphere.c -o ../PC/biosphere")
	#system("mkdir ~/BioData")
}
# scip comments starting with #
$0 ~ /^#/ {next}
{
filename=sprintf("~/BioData/biosphere-%s-%s.csv", $2, date)
if(system("../PC/biosphere " $1 " -s >" filename) != 0)
	message="An Error occured saving Data"
system("echo \"Hallo \"" $2 "\",\n" message \
"im Anhang findest du die neusten Messwerte von deiner Biosphaere.\nMfG, AstroBot\n\n(diese Nachricht wurde automatisch versendet)\" | mail -s \"" \
heading "\" " $3 " -A " filename)
}
' Ports.config
