//Writen by TjarkG and published under the MIT License
//functions to communicate with the biosphere

#ifndef biosphere_HPP_
#define biosphere_HPP_

#include "../reading.h"

long getCommand(const char *cmd);
int setCommand(const char *cmd);
struct reading getReading(char *buf);
void printReading(FILE *ofp, struct reading in);
unsigned int storeReadings(FILE *ofp, bool commenting);
unsigned int bufferReadings(struct reading *buffer);
void printCsvReading(FILE *ofp, struct reading in);
bool setIntervall(unsigned int iNew);
bool syncTime();
bool setOffset(int tIn);
bool setLightTime(time_t time, bool start);
bool setLightThreshold(uint16_t threshold);

#endif //biosphere_HPP_