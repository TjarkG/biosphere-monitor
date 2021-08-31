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

#define HELP "Help.md"          //Name of helpfile
#define OUTFL "biosphere.csv"   //Name of Output File

void syncTime(bool force);
struct reading getReading(void);
struct reading currentReading(void);
void printReading(FILE *ofp, struct reading in);
void printHelp(void);
void storeReadings(void);
long getCsvNumb(char *in);

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
    char *portname = argv[1];
    argc--;
    startUART(portname);

    char i = 2;
    while (--argc > 0)
    {
        if(strncmp(argv[i], "-h", 2) == 0)
            printHelp();
        else if(strncmp(argv[i], "-r", 2) == 0)
        {
            unsigned char buf[32];
            printUART("CR\r");
            getUartLine(buf);
            getUartLine(buf);
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
            printReading(stdout, in);
        }
        else if(strncmp(argv[i], "-s", 2) == 0)
        {
            storeReadings();
        }
        else if(strncmp(argv[i], "-f", 2) == 0)
        {
            unsigned char buf[32];
            time_t rawtime;
            time ( &rawtime );
            sprintf(buf, "TS%ld\r",rawtime);
            printUART(buf);
            getUartLine(buf);

            printUART("TG\r");
            getUartLine(buf);
            getUartLine(buf);
        }
        else if(strncmp(argv[i], "-i", 2) == 0)
        {
            unsigned char buf[32];
            if(strncmp(argv[i], "-i?", 3) == 0)
            {
                printUART("IG\r");
                getUartLine(buf);
                getUartLine(buf);
                printf("Current Messurment intervall:%s",buf);
            }
            else
            {
                sprintf(buf, "IS%s\r",argv[i]+2);
                printUART(buf);
                getUartLine(buf);
                printUART("IG\r");
                getUartLine(buf);
                getUartLine(buf);
                printf("Intervall set:%s Intervall vertified:%s",argv[i]+2,buf);
            }
        }
        else if(strncmp(argv[i], "-t", 2) == 0)
        {
            unsigned char buf[32];
            printUART("DR\r");
            getUartLine(buf);
            getUartLine(buf);
            int error = atoi(buf);
            if(error == 0)
                printf("Self Test passed\n");
            else
                printf("Error detected, Code %d",error);
        }
        else if(strncmp(argv[i], "-g", 2) == 0)
        {
            unsigned char buf[32];
            if(strncmp(argv[i], "-g?", 3) == 0)
            {
                printUART("SG\r");
                getUartLine(buf);
                getUartLine(buf);
                printf("Current Soil Sensor State:%s",buf);
            }
            else
            {
                sprintf(buf, "SS%s\r",argv[i]+2);
                printUART(buf);
                getUartLine(buf);
                printUART("SG\r");
                getUartLine(buf);
                getUartLine(buf);
                printf("Soil Sensor set:%s Soil Sensor vertified:%s",argv[i]+2,buf);
            }
        }
        i++;
    }
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
    while ((in = getc(help)) != EOF)
    {
        printf("%c", in);
    }
    printf("\n\n");
    fclose(help);
}

struct reading getReading(void)
{
    //TODO get next reading from Comport
    struct reading in = {0, 40, 60, 55, 50, 60, 75, 100};
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
    fprintf(out,"Time,Light,T out,T in,Pressure,RH Air,RH Soil,IAQ\n");
    for (int i = 0; i < 10; i++)
    {
        struct reading in = getReading();
        char tmStr[20];
        struct tm lt;
        (void) gmtime_r(&in.timeRead, &lt);
        strftime(tmStr, sizeof(tmStr), "%d.%m.%Y %H:%M:%S", &lt);

        fprintf(out, "%s,%d,%d,%d,%d,%d,%d,%d\n",\
        tmStr, in.light, in.temperaturOut, in.temperaturIn, in.pressure, in.humidityAir, in.humiditySoil, in.iaq);
    }
    fclose(out);
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