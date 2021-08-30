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

#define HELP "Help.md"        //Name of Helpfile

struct reading
{
    time_t timeRead;                //UNIX Timestamp of reading beginning
    unsigned int light;             //outside illuminance       in lux
    unsigned char temperaturOut;    //outside Temperatur        in °C*2
    unsigned char temperaturIN;     //inside Temperatur         in °C*2
    unsigned int pressure;          //inside Pressur            in hPa
    unsigned char humidityAir;      //inside relativ humidity   in %
    signed char humiditySoil;       //inside soil humidity      in %, -1 without a Sensor
    int iaq;                        //Air Quality Index         in IAQ, -1 without Sensor
};

void syncTime(bool force);
struct reading currentReading(void);
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
        if(strcmp(argv[i], "-h") == 0)
        {
            printHelp();
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