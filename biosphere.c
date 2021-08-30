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

#define HELP "Help.md"        //Name of helpfile

void syncTime(bool force);
struct reading currentReading(void);
void printReading(FILE *ofp, struct reading in);
void printHelp(void);
void storeReadings(void);
int getIntervall(void);
void setIntervall(int newInv);
void selfTest(void);
void SoilSens(char *arg);

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
    char *com = argv[1];
    argc--;
    //TODO: Open COM port
    char i = 2;
    while (--argc > 0)
    {
        if(strncmp(argv[i], "-h", 2) == 0)
            printHelp();
        if(strncmp(argv[i], "-r", 2) == 0)
        {
            struct reading in = {0, 40, 60, 55, 50, 60, 75, 100};
            printReading(stdout, in);
        }
        if(strncmp(argv[i], "-s", 2) == 0)
        {}
        if(strncmp(argv[i], "-t", 2) == 0)
        {}
        if(strncmp(argv[i], "-i", 2) == 0)
        {}
        if(strncmp(argv[i], "-t", 2) == 0)
        {}
        if(strncmp(argv[i], "-g", 2) == 0)
        {}
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

void printReading(FILE *ofp, struct reading in)
{
    char tmStr[20];
    struct tm lt;
    (void) gmtime_r(&in.timeRead, &lt);
    strftime(tmStr, sizeof(tmStr), "%d.%m.%Y %H:%M:%S", &lt);
    fprintf(ofp, "Current Reading: Time: %s Outside: %dlux %2.1f°C Inside: %2.1f°C %dhPa, Air: %d%%RH", tmStr, in.light, (in.temperaturOut/2.0), (in.temperaturIn/2.0), in.pressure, in.humidityAir);

    if(in.humiditySoil != -1)
        fprintf(ofp, " Soil: %d%%RH", in.humiditySoil);

    if(in.iaq != -1)
        fprintf(ofp, " %dIAQ", in.iaq);
    fprintf(ofp, "\n");
}