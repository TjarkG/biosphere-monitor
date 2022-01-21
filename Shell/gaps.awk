# run with
# gawk -f Shell/gaps.awk gap=60 Documentation/Sample_Output.csv
# with gap in seconds
BEGIN { FS="[,.: ]" }
$3 ~ /^[2-9]/ {
    #seconds to days
    time = (($1-1)* 86400) + ($4*3600) + ($5*60) + $6
    #years
    time += (($3-1970)*31536000) + (int(($3-1970)/4  + 0.5)*86400)
    #leap days
    if($3%4 == 0 && $2>=3)
        time += 86400
    #months
    switch ($2)
    {
        case 12: time += 2592000
        case 11: time += 2678400
        case 10: time += 2592000
        case 9:  time += 2678400
        case 8:  time += 2678400
        case 7:  time += 2592000
        case 6:  time += 2678400
        case 5:  time += 2592000
        case 4:  time += 2678400
        case 3:  time += 2419200
        case 2:  time += 2678400
    }

    date = $1 "." $2 "." $3 " " $4 ":" $5 ":" $6
    if (timeOld != 0 && time-timeOld != gap)
    {
        print "Gap betwen " dateOld " and " date " (" time-timeOld "s)"
        gaps ++;
    }
    timeOld = time
    dateOld = date
}
END {
    if (gaps != 0)
        print "\n" gaps " Gaps found in " FILENAME
}