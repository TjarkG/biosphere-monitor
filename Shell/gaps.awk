# run with
# awk -f Shell/gaps.awk gap=60 Documentation/Sample_Output.csv
# with gap in seconds
BEGIN { FS="[,.]" }
#change for 11. Century
$3 ~ /^[2-9]/ {
    cmd = "date --date='" $2 "/" $1 "/" $3 "' +\"%s\" -u"
    if ((cmd | getline time) != 1)
    {
        print "Error reading: " $0 " with " cmd
        time = -1
    }
    if (timeOld != 0 && time-timeOld != gap)
    {
        print "Gap betwen " dateOld " and " $1 "." $2 "." $3 " (" time-timeOld "s)"
        gaps ++;
    }
    timeOld = time
    dateOld = $1 "." $2 "." $3
}
END {
    if (gaps != 0)
    {
        print "\n" gaps " Gaps found in " FILENAME
    }
}