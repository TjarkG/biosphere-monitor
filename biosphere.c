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
#include "reading.h"
#include "tty.h"

#define HELP "README.md"          //Name of helpfile
#define OUTFL "biosphere.csv"   //Name of Output File
#define ESC 27

long getCommand(const char *cmd);
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
        unsigned char buf[32];
        if(strncmp(argv[i], "-h", 2) == 0)
            printHelp();
        else if(strncmp(argv[i], "-r", 2) == 0)
        {
            printUART("CR\r");
            getUartLine(buf);
            getUartLine(buf);
            printReading(stdout, getReading(buf));
        }
        else if(strncmp(argv[i], "-s", 2) == 0)
            storeReadings();
        else if(strncmp(argv[i], "-f", 2) == 0)
        {
            time_t rawtime;
            time ( &rawtime );
            sprintf(buf, "TS%ld\r",rawtime);
            printUART(buf);
            getUartLine(buf);

            printUART("TG\r");
            getUartLine(buf);
            getUartLine(buf);
            printf("Syncronized System Times\n");
        }
        else if(strncmp(argv[i], "-i?", 3) == 0)
        {
            printUART("IG\r");
            getUartLine(buf);
            getUartLine(buf);
            printf("Current Messurment intervall:%s",buf);
        }
        else if(strncmp(argv[i], "-i", 2) == 0)
        {
            sprintf(buf, "IS%s\r",argv[i]+2);
            printUART(buf);
            getUartLine(buf);
            printUART("IG\r");
            getUartLine(buf);
            getUartLine(buf);
            printf("Intervall set:%s Intervall vertified:%s",argv[i]+2,buf);
        }
        else if(strncmp(argv[i], "-t", 2) == 0)
        {
            printUART("DR\r");
            getUartLine(buf);
            getUartLine(buf);
            int error = atoi(buf);
            if(error == 0)
                printf("Self Test passed\n");
            else
                printf("Error detected, Code %d",error);
        }
        else if(strncmp(argv[i], "-g?", 3) == 0)
        {
            /*printUART("SG\r");
            getUartLine(buf);
            getUartLine(buf);
            printf("Current Soil Sensor State: %s",buf);*/
            printf("Current Soil Sensor State: %d",getCommand("SG"));
        }
        else if(strncmp(argv[i], "-g", 2) == 0)
        {
            sprintf(buf, "SS%s\r",argv[i]+2);
            printUART(buf);
            getUartLine(buf);
            printUART("SG\r");
            getUartLine(buf);
            getUartLine(buf);
            printf("Soil Sensor set:%s Soil Sensor vertified:%s",argv[i]+2,buf);
        }
        i++;
    }
}

long getCommand(const char *cmd)       //send get command and put return response
{
    unsigned char buf[32];
    printUART(cmd);
    printUART("\r");
    getUartLine(buf);
    if(strcmp(buf, cmd))
    {
        fprintf(stderr, "Error tranmitting UART Command %s: recieved %s", cmd, buf);
        return -1;
    }
    getUartLine(buf);
    return atoi(buf);
}

void printHelp(void)
{
    FILE *help;
    char in;

    if ((help = fopen(HELP, "r")) == NULL) 
    {
        fprintf(stderr, "can't open %s\n", HELP);
        exit(1);
    }
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
    struct reading in = {0, 0, 0, 0, 0, 0, -1, -1};
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
        tmStr, in.light, in.temperaturOut/2.0, in.temperaturIn/2.0, in.pressure, in.humidityAir, in.humiditySoil, in.iaq);
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
    fprintf(ofp, "Current Reading: Time: %s UTC Outside: %dlux %2.1f°C Inside: %2.1f°C %dhPa, Air: %d%%RH",\
    tmStr, in.light, (in.temperaturOut/2.0), (in.temperaturIn/2.0), in.pressure, in.humidityAir);

    if(in.humiditySoil != -1)
        fprintf(ofp, " Soil: %d%%RH", in.humiditySoil);

    if(in.iaq != -1)
        fprintf(ofp, " %dIAQ", in.iaq);
    fprintf(ofp, "\n");
}