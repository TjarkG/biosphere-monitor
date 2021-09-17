/*
 * biosphere-monitor\biosphere.c
 *
 * command line tool to interface with the biosphere monitor board
 *
 * Created: 30.08.2021 08:23:42
 * Author : Tjark Gaudich
 */ 

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <string.h>
#include "../reading.h"
#include "tty.h"

#define HELP "README.md"          //Name of helpfile
#define OUTFL "biosphere.csv"   //Name of Output File
#define ESC 27

long getCommand(const char *cmd);
int setCommand(const char *cmd);
struct reading getReading(char *buf);
void printReading(FILE *ofp, struct reading in);
void printHelp(void);
void storeReadings(void);

int main(int argc, char *argv[])
{
    char *prog = argv[0];     // program name for errors

    if(strcmp(argv[1], "-h") == 0)
        printHelp();
    if (argc == 1 || argv[1][0] == '-') /* no args: throw error */
    {
        fprintf(stderr, "%s: first argument must be target COM Port\n", prog);
        return -1;
    }
    argc--;
    startUART(argv[1]);

    char i = 2;
    while (--argc > 0)
    {
        unsigned char buf[64];
        if(strncmp(argv[i], "-h", 2) == 0)
            printHelp();
        else if(strncmp(argv[i], "-r", 2) == 0)
        {
            setCommand("CR");
            getUartLine(buf);
            printReading(stdout, getReading(buf));
        }
        else if(strncmp(argv[i], "-s", 2) == 0)
            storeReadings();
        else if(strncmp(argv[i], "-f", 2) == 0)
        {
            time_t rawtime;
            time ( &rawtime );
            sprintf(buf, "TS%ld",rawtime);
            setCommand(buf);
            printf("Syncronized System Times\n");
        }
        else if(strncmp(argv[i], "-i?", 3) == 0)
            printf("Current Messurment intervall: %li\n",getCommand("IG"));
        else if(strncmp(argv[i], "-i", 2) == 0)
        {
            sprintf(buf, "IS%s",argv[i]+2);
            setCommand(buf);
            printf("Intervall set:%s Intervall vertified:%ld\n",argv[i]+2,getCommand("IG"));
        }
        else if(strncmp(argv[i], "-t", 2) == 0)
        {
            int error = getCommand("DR");
            if(error == 0)
                printf("Self Test passed\n");
            else
                printf("Error detected, Code %d\n",error);
        }
        else if(strncmp(argv[i], "-g?", 3) == 0)
            printf("Current Soil Sensor State: %li\n",getCommand("SG"));
        else if(strncmp(argv[i], "-g", 2) == 0)
        {
            sprintf(buf, "SS%s",argv[i]+2);
            setCommand(buf);
            printf("Soil Sensor set:%s Soil Sensor vertified:%ld\n",argv[i]+2,getCommand("SG"));
        }
        else if(strncmp(argv[i], "-ct", 3) == 0)
        {
            int tIn = atof(argv[i]+3)*5;

            setCommand("CR");
            getUartLine(buf);
            struct reading in = getReading(buf);

            int offOld = getCommand("OGT");

            int off = tIn - (in.temperaturOut-offOld+128) + 128;
            sprintf(buf, "OST%d",off);
            setCommand(buf);
            printf("Outside Temperatur set:%2.1f°C Old Offset: %d New Offset:%d Offset Vertified: %ld\n",tIn/5.0, offOld-128, off-128, getCommand("OGT")-128);

            offOld = getCommand("OGI");

            off = tIn - (in.temperaturIn-offOld+128) + 128;
            sprintf(buf, "OSI%d",off);
            setCommand(buf);
            printf("Inside Temperatur set:%2.1f°C Old Offset: %d New Offset:%d Offset Vertified: %ld\n",tIn/5.0, offOld-128, off-128, getCommand("OGI")-128);
        }
        i++;
    }
}

long getCommand(const char *cmd)       //send get command and return response
{
    unsigned char buf[16];
    setCommand(cmd);
    getUartLine(buf);
    return atoi(buf);
}
int setCommand(const char *cmd)       //send set command
{
    unsigned char buf[16];
    printUART(cmd);
    printUART("\r");
    getUartLine(buf);
    if(strncmp(buf, cmd,strlen(cmd)))
    {
        fprintf(stderr, "Error tranmitting UART Command %s: recieved %s", cmd, buf);
        return -1;
    }
    return 0;
}

void printHelp(void)
{
    FILE *help;
    if ((help = fopen(HELP, "r")) == NULL) 
    {
        fprintf(stderr, "can't open %s\n", HELP);
        exit(1);
    }

    char in;
    bool header = false;
    while ((in = getc(help)) != EOF)
    {
        if(in == '#')
        {
            printf("%c[1m",ESC);
            getc(help);
            header = true;
        }
        else if(in == '\n' && header)
        {
            printf("%c[0m",ESC);
            header = false;
        }
        else
            printf("%c", in);
    }
    printf("\n\n");
    fclose(help);
}

struct reading getReading(char *buf)
{
    struct reading in;
    int i, count;
    for (i=0, count=0; buf[i]; i++)     //count number of ocurences of ',' in input String to prevent memory acces errors
        count += (buf[i] == ',');
    if(count != 7)
    {
        fprintf(stderr, "Error getting Reading from input \"%s\" only %d of 7 ',' found\n",buf, count);
        return in;
    }
    char *ptr = strtok(buf, ",\n");
    in.timeRead = atol(ptr);
    ptr = strtok(NULL, ",\n");
    in.light = atoi(ptr);
    ptr = strtok(NULL, ",\n");
    in.temperaturOut = atoi(ptr);
    ptr = strtok(NULL, ",\n");
    in.temperaturIn = atoi(ptr);
    ptr = strtok(NULL, ",\n");
    in.pressure = atoi(ptr);
    ptr = strtok(NULL, ",\n");
    in.humidityAir = atoi(ptr);
    ptr = strtok(NULL, ",\n");
    in.humiditySoil = atoi(ptr);
    ptr = strtok(NULL, ",\n");
    in.iaq = atoi(ptr);
    return in;
}

void storeReadings(void)
{
    FILE *out;
    if ((out = fopen(OUTFL, "w")) == NULL) 
    {
        fprintf(stderr, "can't open %s\n", OUTFL);
        exit(1);
    }
    printf("Startet Saving Readings...\n");
    fprintf(out,"UTC,Light,°C out,°C in,hPa,RH Air,RH Soil,IAQ\n");
    unsigned char buf[32];
    printUART("AR\r");
    getUartLine(buf);
    unsigned int lnCnt = 0;
    while(1)
    {
        getUartLine(buf);
        if(strncmp(buf, "EOF",3) == 0)
            break;
        struct reading in = getReading(buf);
        char tmStr[20];
        struct tm lt;
        (void) gmtime_r(&in.timeRead, &lt);
        strftime(tmStr, sizeof(tmStr), "%d.%m.%Y %H:%M:%S", &lt);

        fprintf(out, "%s,%d,%2.1f,%2.1f,%d,%d,%d,%d\n",\
        tmStr, in.light, in.temperaturOut/5.0, in.temperaturIn/10.0, in.pressure, in.humidityAir, in.humiditySoil, in.iaq);
        lnCnt++;
    }
    fclose(out);
    printf("Finished! %u Readings Saved\n",lnCnt);
}

void printReading(FILE *ofp, struct reading in)
{
    char tmStr[20];
    struct tm lt;
    (void) gmtime_r(&in.timeRead, &lt);
    strftime(tmStr, sizeof(tmStr), "%d.%m.%Y %H:%M:%S", &lt);

    fprintf(ofp, "Current Reading: Time: %s UTC Outside: %dlux %2.1f°C Inside: %2.1f°C %dhPa",\
    tmStr, in.light, (in.temperaturOut/5.0), (in.temperaturIn/10.0), in.pressure);

    if(in.humidityAir != 0)
        fprintf(ofp, ", Air: %d%%RH", in.humidityAir);
    if(in.humiditySoil != 0)
        fprintf(ofp, " Soil: %d%%RH", in.humiditySoil);
    if(in.iaq != 0)
        fprintf(ofp, " %dIAQ", in.iaq);
    
    fprintf(ofp, "\n");
}