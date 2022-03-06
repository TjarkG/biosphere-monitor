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
#define ESC 27

static char *errCodes[] = {"UART Transmittion", "AVCC", "RTC running", "RTC initialized", "Flash Signatur", "Flash erase", "Flash read/write", "UART Tx level",
"UART Rx level", "Outside Temperatur", "Light Sensor", "Intervall set", "Temperatur offset set", "BME connected", "BME Readings in range"};

long getCommand(const char *cmd);
int setCommand(const char *cmd);
struct reading getReading(char *buf);
void printReading(FILE *ofp, struct reading in);
void printHelp(void);
void storeReadings(FILE *ofp, bool commenting);
void printCsvReading(FILE *ofp, struct reading in);
bool setIntervall(unsigned int iNew);
bool synctime(void);
bool setOffset(int tIn);

int main(int argc, char *argv[])
{
    char *prog = argv[0];     // program name for errors

    if(strcmp(argv[1], "-h") == 0)
        printHelp();
    if (argc == 1 || argv[1][0] == '-') /* no args or arguments cant be a serial Port: throw error */
    {
        fprintf(stderr, "%s: first argument must be target COM Port\n", prog);
        return -1;
    }
    argc--;
    startUART(argv[1]);

    char i = 2;
    while (--argc > 0)
    {
        unsigned char buf[128];
        if(strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "-help") == 0)
            printHelp();
        else if(strcmp(argv[i], "-r") == 0)
        {
            setCommand("CR");
            getUartLine(buf);
            struct reading in = getReading(buf);
            printReading(stdout, in);
        }
        else if(strcmp(argv[i], "-rm") == 0)
        {
            setCommand("CR");
            getUartLine(buf);
            struct reading in = getReading(buf);
            printCsvReading(stdout, in);
        }
        else if(strcmp(argv[i], "-s") == 0)
        {
            storeReadings(stdout, false);
        }
        else if(strcmp(argv[i], "-sc") == 0)
        {
            storeReadings(stdout, true);
        }
        else if(strcmp(argv[i], "-f") == 0)
        {
            if(synctime())
            {
                fprintf(stderr, "first syncronization atempt faild, trying again\n");
                if(synctime())
                    fprintf(stderr, "Time Syncronization failed\n");
                else
                    fprintf(stderr, "Syncronized System Times on second attempt\n");
            }
        }
        else if(strcmp(argv[i], "-i?") == 0)
            printf("Messurment intervall: %li\n",getCommand("IG"));
        else if(strncmp(argv[i], "-i", 2) == 0)
        {
            if(setIntervall(atoi(argv[i]+2)))
                fprintf(stderr, "Intervall sucessfuly set\n");
            else
                fprintf(stderr, "an Error ocured setting intervall\n");
        }
        else if(strcmp(argv[i], "-t") == 0)
        {
            int error = getCommand("DR");

            for (int i = 0; i < (sizeof(errCodes) / sizeof(errCodes[0])); i++)
            {
                printf("%-24s%s\n", errCodes[i], (error & (1 << i)) ? "Error": "Ok");
            }
            
            if(error == 0)
                printf("\nSelf Test passed\n");
            else
                printf("\nError detected, Code %d\n",error);
        }
        else if(strcmp(argv[i], "-delete") == 0)
        {
            setCommand("DEL");
        }
        else if(strncmp(argv[i], "-ct", 3) == 0)
        {
            if(setOffset(atof(argv[i]+3)*5))
                fprintf(stderr, "Temperature Offset set\n");
            else
                fprintf(stderr, "An Error ocured setting Temperature Offset\n");
        }
        else if(strcmp(argv[i], "-gh") == 0)
        {
            printf("%d\n",(unsigned int) getCommand("GH"));
        }
        else
            fprintf(stderr, "Unknow Argument: %s\n", argv[i]);
        i++;
    }
    stopUART();
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
        fprintf(stderr, "Error tranmitting UART Command %s: recieved %s\n", cmd, buf);
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

void storeReadings(FILE *ofp, bool commenting)
{
    if(commenting)
        fprintf(stderr, "Startet Saving Readings...\n");
    printf("UTC,Light,°C out,°C in,hPa,RH Air,RH Soil,IAQ\n");
    unsigned char buf[64];
    printUART("AR\r");
    getUartLine(buf);
    unsigned long lnCnt = 0;
    while(1)
    {
        getUartLine(buf);
        if(buf[0] == 'E')       //detecting EOF with a string comperasions somehow made the whole program slower than the Microcontroller is transmitting
            break;
        struct reading in = getReading(buf);
        printCsvReading(ofp, in);
        
        if(lnCnt%250 == 0 && commenting)
        {
            putc('#', stderr);
            fflush(stderr);
        }
        lnCnt++;
    }
    if(commenting)
        fprintf(stderr, "\nFinished! %lu Readings Saved\n",lnCnt);
}

void printReading(FILE *ofp, struct reading in)
{
    char tmStr[20];
    struct tm lt;
    lt = *gmtime(&in.timeRead);
    strftime(tmStr, sizeof(tmStr), "%d.%m.%Y %H:%M:%S", &lt);

    fprintf(ofp, "Current Reading: Time: %s UTC Outside: %dlux %2.1fC Inside: %2.1fC %dhPa",\
    tmStr, in.light, (in.temperaturOut/5.0), (in.temperaturIn/10.0), in.pressure);

    if(in.humidityAir != 0)
        fprintf(ofp, ", Air: %d%%RH", in.humidityAir);
    if(in.humiditySoil != 0)
        fprintf(ofp, " Soil: %d%%RH", in.humiditySoil);
    if(in.iaq != 0)
        fprintf(ofp, " %dIAQ", in.iaq);
    
    fprintf(ofp, "\n");
}

void printCsvReading(FILE *ofp, struct reading in)
{
    char tmStr[20];
    struct tm lt;
    lt = *gmtime(&in.timeRead);
    strftime(tmStr, sizeof(tmStr), "%d.%m.%Y %H:%M:%S", &lt);

    fprintf(ofp,"%s,%d,%d.%d,%d.%d,%d,%d,%d,%d\n",\
    tmStr, in.light, in.temperaturOut/5, 2*(in.temperaturOut%5), in.temperaturIn/10, in.temperaturIn%10, in.pressure, in.humidityAir, in.humiditySoil, in.iaq);
}

bool setIntervall(unsigned int iNew)
{
    unsigned char buf[16];
    sprintf(buf, "IS%d",iNew);
    setCommand(buf);
    unsigned int iVert = getCommand("IG");
    return (iNew == iVert);
}

bool synctime(void)
{
    unsigned char buf[32];
    time_t rawtime;
    time ( &rawtime );
    sprintf(buf, "TS%ld",rawtime);
    setCommand(buf);
    return abs(rawtime - getCommand("TG")) > 3;
}

bool setOffset(int tIn)
{
    unsigned char buf[64];
    setCommand("CR");
    getUartLine(buf);
    struct reading in = getReading(buf);

    int offOld = getCommand("OGT");

    int off = tIn - (in.temperaturOut-offOld+128) + 128;
    sprintf(buf, "OST%d",off);
    setCommand(buf);
    return off == getCommand("OGT");
}