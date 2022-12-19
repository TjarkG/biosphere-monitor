//Writen by TjarkG and published under the MIT License
//command line tool to interface with the biosphere monitor board

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <string.h>
#include <iostream>
#include "../reading.h"
#include "tty.h"
#include "biosphere.h"

#include "README.md.hpp"

#define HELP "README.md"          //Name of helpfile
#define ESC 27

static std::string errCodes[] = {"UART Transmission", "AVCC", "RTC running", "RTC initialized", "Flash Signatur", "Flash erase", "Flash read/write", "UART Tx level",
"UART Rx level", "Outside Temperatur", "Light Sensor", "Intervall set", "Temperatur offset set", "BME connected", "BME Readings in range"};

void printT(time_t time)
{
    printf("%02ld:%02ld:%02ld\n", time/3600, (time%3600)/60, time%60);
}

//get time in seconds after midnight from hh:mm:ss, hh:mm or hh timeformat at the start of str
time_t getTime(char *str)
{
    time_t time = 3600 * strtol(str, &str, 10);
    time += 60 * strtol(str+1, &str, 10);
    time += strtol(str+1, NULL, 10);

    return time;
}

int main(int argc, char *argv[])
{
    char *prog = argv[0];     // program name for errors

    if (argc == 1 || (argv[1][0] == '-' && strcmp(argv[1], "-h")) != 0) /* no args or arguments cant be a serial Port: throw error */
    {
        fprintf(stderr, "%s: first argument must be target COM Port\n", prog);
        return -1;
    }
    if(strcmp(argv[1], "-h") == 0)
    {
        std::cout << README;
        return 0;
    }
    argc--;
    if(startUART(argv[1]) != 0)
    {
        fprintf(stderr, "Can't open Port %s, exiting\n", argv[1]);
        return -2;
    }

    char i = 2;
    while (--argc > 0)
    {
        char buf[128];
        if(strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "-help") == 0)
            std::cout << README;
        else if(strcmp(argv[i], "-r") == 0)
        {
            if(setCommand("CR") < 0)
                exit(2);
            getUartLine(buf);
            struct reading in = getReading(buf);
            printReading(stdout, in);
        }
        else if(strcmp(argv[i], "-rm") == 0)
        {
            if(setCommand("CR") < 0)
                exit(2);
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
                printf("%-24s%s\n", errCodes[i].c_str(), (error & (1 << i)) ? "Error": "Ok");
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

        else if(strcmp(argv[i], "-ltn?") == 0)
        {
            printf("Ligth on time: ");
            printT(getCommand("GLN"));
        }
        else if(strncmp(argv[i], "-ltn", 4) == 0)
        {
            if(setLightTime(getTime(argv[i]+4), true))
                fprintf(stderr, "Light on time sucessfuly set\n");
            else
                fprintf(stderr, "an Error ocured setting Light on time\n");
        }
        else if(strcmp(argv[i], "-ltf?") == 0)
        {
            printf("Ligth off time: ");
            printT(getCommand("GLF"));
        }
        else if(strncmp(argv[i], "-ltf", 4) == 0)
        {
            if(setLightTime(getTime(argv[i]+4), false))
                fprintf(stderr, "Light off time sucessfuly set\n");
            else
                fprintf(stderr, "an Error ocured setting Light off time\n");
        }
        else if(strcmp(argv[i], "-lt?") == 0)
        {
            int treshold = getCommand("GLT");
            if(treshold == 0)
                printf("Light treshold off\n");
            else
                printf("Light treshold: %d Lux\n", treshold);
        }
        else if(strncmp(argv[i], "-lt", 3) == 0)
        {
            if(setLightTreshold(atoi(argv[i]+3)))
                fprintf(stderr, "Light Treshold sucessfuly set\n");
            else
                fprintf(stderr, "an Error ocured setting Light Treshold\n");
        }

        else
            fprintf(stderr, "Unknow Argument: %s\n", argv[i]);
        i++;
    }
    stopUART();
}