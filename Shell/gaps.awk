BEGIN { FS="[,.]" }
{time = system("date --date='" $2 "/" $1 "/" $3 "' +\"%s\" -u") 
print time
}