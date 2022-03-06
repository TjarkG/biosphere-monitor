/*
 * reading.h
 *
 * Header with struct to store Sensor redings in on PC and MC
 *
 * Created: 30.08.2021 16:37:17
 *  Author: Tjark Gaudich
 */

#ifndef reading_H_
#define reading_H_

struct reading
{
    time_t timeRead;                //UNIX Timestamp of reading beginning
    unsigned int light;             //outside illuminance       in lux
    unsigned char temperaturOut;    //outside Temperatur        in °C*5
    unsigned int temperaturIn;      //inside Temperatur         in °C*10
    unsigned int pressure;          //inside Pressur            in hPa
    unsigned char humidityAir;      //inside relativ humidity   in %
    unsigned char humiditySoil;     //inside soil humidity      in %, 0 without a Sensor
    int iaq;                        //Air Quality Index         in IAQ, 0 without Sensor
}; 

#endif //reading_H_