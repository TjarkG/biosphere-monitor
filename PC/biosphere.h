//Writen by TjarkG and published under the MIT License

#ifndef biosphere_H_
#define biosphere_H_

#include <stdio.h>
#include <stdbool.h>

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

#endif