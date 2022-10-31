//Writen by TjarkG and published under the MIT License
//functions to communicate with the biosphere

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <string.h>
#include "../reading.h"
#include "tty.h"
#include "biosphere.h"

long getCommand(const char *cmd)       //send get command and return response
{
    char buf[16];
    if (setCommand(cmd) == 0)
    {
        getUartLine(buf);
        return atoi(buf);
    }
    else
        return -1;
}

int setCommand(const char *cmd)       //send set command
{
    char buf[16];
    printUART(cmd);
    printUART("\r");
    char err = getUartLine(buf);
    if(strncmp(buf, cmd,strlen(cmd)) || err)
    {
        fprintf(stderr, "Error tranmitting UART Command \"%s\": recieved \"%s\"\n", cmd, buf);
        return -1;
    }
    return 0;
}

struct reading getReading(char *buf)
{
    struct reading in = {0};
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
    return in;
}

unsigned int storeReadings(FILE *ofp, bool commenting)
{
    if(commenting)
        fprintf(stderr, "Startet Saving Readings...\n");
    fprintf(ofp, "UTC,Light,°C out,°C in,hPa,RH Air,RH Soil\n");
    char buf[64];
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
    return lnCnt;
}

unsigned int bufferReadings(struct reading *buffer) //this assumes buffer is bigger than required
{
    char buf[64];
    printUART("AR\r");
    getUartLine(buf);
    unsigned long lnCnt = 0;
    while(1)
    {
        getUartLine(buf);
        if(buf[0] == 'E')       //detecting EOF with a string comperasions somehow made the whole program slower than the Microcontroller is transmitting
            break;
        buffer[lnCnt++] = getReading(buf);
    }
    return lnCnt;
}

void printReading(FILE *ofp, struct reading in)
{
    char tmStr[20];
    struct tm lt;
    lt = *gmtime(&in.timeRead);
    strftime(tmStr, sizeof(tmStr), "%d.%m.%Y %H:%M:%S", &lt);

    fprintf(ofp, "Current Reading: Time: %s UTC Outside: %dlux %2.1fC Inside: %2.1fC %2.1fhPa",\
    tmStr, in.light, in.temperaturOut/(double) SCALE_TOut, in.temperaturIn/(double) SCALE_TIn, in.pressure/(double) SCALE_Pres);

    if(in.humidityAir != 0)
        fprintf(ofp, ", Air: %d%%RH", in.humidityAir);
    if(in.humiditySoil != 0)
        fprintf(ofp, " Soil: %d%%RH", in.humiditySoil);
    
    fprintf(ofp, "\n");
}

void printCsvReading(FILE *ofp, struct reading in)
{
    char tmStr[20];
    struct tm lt;
    lt = *gmtime(&in.timeRead);
    strftime(tmStr, sizeof(tmStr), "%d.%m.%Y %H:%M:%S", &lt);

    fprintf(ofp,"%s,%d,%2.1f,%2.1f,%2.1f,%d,%d\n",\
    tmStr, in.light, in.temperaturOut/(double) SCALE_TOut, in.temperaturIn/(double) SCALE_TIn, in.pressure/(double) SCALE_Pres, in.humidityAir, in.humiditySoil);
}

bool setIntervall(unsigned int iNew)
{
    if(iNew >= 65536)
        return false;
    char buf[16];
    sprintf(buf, "IS%d",iNew);
    setCommand(buf);
    //usleep(50000);
    //TODO: Chrono
    unsigned int iVert = getCommand("IG");
    return (iNew == iVert);
}

bool synctime(void)
{
    char buf[32];
    time_t rawtime;
    time ( &rawtime );
    sprintf(buf, "TS%ld",rawtime);
    setCommand(buf);
    return labs(rawtime - getCommand("TG")) > 3;
}

bool setOffset(int tIn)
{
    char buf[64];
    setCommand("CR");
    getUartLine(buf);
    struct reading in = getReading(buf);

    int offOld = getCommand("OGT");

    int off = tIn - (in.temperaturOut-offOld+128) + 128;
    sprintf(buf, "OST%d",off);
    setCommand(buf);
    return off == getCommand("OGT");
}

bool setLightTime(const time_t time, const bool start)
{
    char buf[16];
    sprintf(buf, "SL%c%ld", start ? 'N' : 'F', time);
    setCommand(buf);


    sprintf(buf, "GL%c", start ? 'N' : 'F');
    return time == getCommand(buf);
}

bool setLightTreshold(const uint16_t treshold)
{
    char buf[16];
    sprintf(buf, "SLT%d", treshold);
    setCommand(buf);
    
    return treshold == getCommand("GLT");
}