/*
 * reading.h
 *
 * Header with struct to store Sensor readings in on PC and MC
 *
 * Created: 30.08.2021 16:37:17
 *  Author: Tjark Gaudich
 */

#ifndef reading_H_
#define reading_H_

#include <time.h>
#include <stdint.h>

#define SCALE_TOut  5
#define SCALE_TIn   10
#define SCALE_Pres  10

struct reading
{
    time_t timeRead;            //UNIX Timestamp of reading beginning
    uint16_t light;             //outside illuminance       in lux
    uint8_t temperaturOut;      //outside Temperatur        in °C*5
    uint16_t temperaturIn;      //inside Temperatur         in °C*10
    uint16_t pressure;          //inside Pressure            in hPa*10
    uint8_t humidityAir;        //inside relativ humidity   in %
    uint8_t humiditySoil;       //inside soil humidity      in %, 0 without a Sensor
}; 

#endif //reading_H_