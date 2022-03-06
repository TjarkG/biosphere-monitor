//Writen by TjarkG and published under the MIT License
//functions to communicate with the biosphere

#ifndef biosphere_H_
#define biosphere_H_

#include <stdio.h>
#include <stdbool.h>
#include "../reading.h"

long getCommand(const char *cmd);
int setCommand(const char *cmd);
struct reading getReading(char *buf);
void printReading(FILE *ofp, struct reading in);
void printHelp(void);
unsigned int storeReadings(FILE *ofp, bool commenting);
unsigned int bufferReadings(struct reading *buffer);
void printCsvReading(FILE *ofp, struct reading in);
bool setIntervall(unsigned int iNew);
bool synctime(void);
bool setOffset(int tIn);

#endif