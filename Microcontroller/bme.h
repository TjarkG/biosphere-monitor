//Writen by TjarkG and published under the MIT License

#ifndef bme_H_
#define bme_H_

#include <avr/io.h>
#include <util/delay.h>

char id;        //Sensor ID

char bmeInit(void);
unsigned int getBmeTemp(void);
unsigned int getBmePress(void);
unsigned char getBmeHumidity(void);
int getBmeIaq(void);

#endif //bme_H_