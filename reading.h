/*
 * reading.h
 *
 * Header with struct to store Sensor redings in on PC and MC
 *
 * Created: 30.08.2021 16:37:17
 *  Author: Tjark Gaudichs
 */

#ifndef reading_H_
#define reading_H_

struct reading
{
    time_t timeRead;                //UNIX Timestamp of reading beginning
    unsigned char light;            //outside illuminance       in ?
    unsigned char temperaturOut;    //outside Temperatur        in °C*2
    unsigned char temperaturIn;     //inside Temperatur         in °C*2
    unsigned int pressure;          //inside Pressur            in hPa
    unsigned char humidityAir;      //inside relativ humidity   in %
    signed char humiditySoil;       //inside soil humidity      in %, -1 without a Sensor
    int iaq;                        //Air Quality Index         in IAQ, -1 without Sensor
}; 

#endif //reading_H_