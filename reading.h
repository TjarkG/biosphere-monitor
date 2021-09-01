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

long readingIt(struct reading *v, char i) //makes it possible to itterate throuh a reading
{
    switch(i) 
    {
        case 0: return v->timeRead;
        case 1: return v->light;
        case 2: return v->temperaturOut;
        case 3: return v->temperaturIn;
        case 4: return v->pressure;
        case 5: return v->humidityAir;
        case 6: return v->humiditySoil;
        case 7: return v->iaq;
    }
    return 0;
}

#endif //reading_H_