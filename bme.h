/*
 * bme.h
 *
 * Header to communicate with a BME series Sensor on TWIC
 *
 * Created: 04.09.2021 21:01:42
 *  Author: Tjark Gaudich
 */

#ifndef bme_H_
#define bme_H_

#ifndef __AVR_ATxmega16A4U__
#define __AVR_ATxmega16A4U__
#endif

#include <avr/io.h>
#include <util/delay.h>

long t_fine;    //Fine Temperatur for Pressur Compensation

struct cal    //compensation Values
{
    unsigned short T1;
    signed short T2;
    signed short T3;
    unsigned short P1;
    signed short P2;
    signed short P3;
    signed short P4;
    signed short P5;
    signed short P6;
    signed short P7;
    signed short P8;
    signed short P9;
} dig;

char bmeInit(void);
unsigned char bmeReadRegister(const char reg);
void bmeWriteRegister(const char reg, const unsigned char data);
void bmeSelectReg(const char reg);
unsigned short getBmeTemp(void);
unsigned int getBmePress(void);

char bmeInit(void)
{
    TWIC.MASTER.BAUD = 155;
    TWIC.MASTER.CTRLA = TWI_MASTER_ENABLE_bm;
    TWIC.MASTER.STATUS |= TWI_MASTER_BUSSTATE_IDLE_gc;

    char id = bmeReadRegister(0xD0);      //Get Device ID

    dig.T1 = bmeReadRegister(0x88);
    dig.T1 |= bmeReadRegister(0x89)<<8;
    dig.T2 = bmeReadRegister(0x8A);
    dig.T2 |= bmeReadRegister(0x8B)<<8;
    dig.T3 = bmeReadRegister(0x8C);
    dig.T3 |= bmeReadRegister(0x8D)<<8;
    dig.P1 = bmeReadRegister(0x8E);
    dig.P1 |= bmeReadRegister(0x8F)<<8;
    dig.P2 = bmeReadRegister(0x90);
    dig.P2 |= bmeReadRegister(0x91)<<8;
    dig.P3 = bmeReadRegister(0x92);
    dig.P3 |= bmeReadRegister(0x93)<<8;
    dig.P4 = bmeReadRegister(0x94);
    dig.P4 |= bmeReadRegister(0x95)<<8;
    dig.P5 = bmeReadRegister(0x96);
    dig.P5 |= bmeReadRegister(0x97)<<8;
    dig.P6 = bmeReadRegister(0x98);
    dig.P6 |= bmeReadRegister(0x99)<<8;
    dig.P7 = bmeReadRegister(0x9A);
    dig.P7 |= bmeReadRegister(0x9B)<<8;
    dig.P8 = bmeReadRegister(0x9C);
    dig.P8 |= bmeReadRegister(0x9D)<<8;
    dig.P9 = bmeReadRegister(0x9E);
    dig.P9 |= bmeReadRegister(0x9F)<<8;

    switch (id)
    {
    case 0x58: return 1; break; //BMP280
    case 0x60: return 2; break; //BME280
    }
    return 0;
}

unsigned char bmeReadRegister(const char reg)
{
    bmeSelectReg(reg);
    TWIC.MASTER.CTRLC = TWI_MASTER_CMD_STOP_gc;

    TWIC.MASTER.ADDR = (0x76 << 1) | 0x01;
    while(!((TWIC.MASTER.STATUS & TWI_MASTER_RIF_bm)));
    unsigned char data = 0;
    while(!(TWIC.MASTER.STATUS & TWI_MASTER_RIF_bm));
    data = TWIC.MASTER.DATA;
    TWIC.MASTER.CTRLC = TWI_MASTER_ACKACT_bm | TWI_MASTER_CMD_STOP_gc;
    return data;
}

void bmeWriteRegister(const char reg, const unsigned char data)
{
    bmeSelectReg(reg);
    while(!(TWIC.MASTER.STATUS & TWI_MASTER_WIF_bm));
    TWIC.MASTER.CTRLC = TWI_MASTER_CMD_STOP_gc;
}

void bmeSelectReg(const char reg)
{
    TWIC.MASTER.ADDR = 0x76 << 1;
    while(!((TWIC.MASTER.STATUS & TWI_MASTER_WIF_bm)));
    TWIC.MASTER.DATA = reg;
    while(!(TWIC.MASTER.STATUS & TWI_MASTER_WIF_bm));
}

unsigned short getBmeTemp(void)  //returns BME Temperatur in °C*10
{
    long data = 0;
    bmeWriteRegister(0xF4, 0x01 | (0b0011 << 2) | (0b0001 << 5));
    _delay_ms(20);
    data |= ((long)bmeReadRegister(0xFA) << 12);
    data |= ((long)bmeReadRegister(0xFB) << 8);
    data |= ((long)bmeReadRegister(0xFC) << 0);
    long var1, var2;
    var1  = ((((data>>3) - ((long)dig.T1<<1))) * ((long)dig.T2)) >> 11;
    var2  = (((((data>>4) - ((long)dig.T1)) * ((data>>4) - ((long)dig.T1))) >> 12) * ((long)dig.T3)) >> 14;
    t_fine = var1 + var2;
    return ((t_fine * 5 + 128) >> 8)/10;
}

unsigned int getBmePress(void)  //returns BME Pressure in hPa
{
    long data = 0;
    bmeWriteRegister(0xF4, 0x01 | (0b0011 << 2) | (0b0001 << 5));
    _delay_ms(20);
    data |= ((long)bmeReadRegister(0xF7) << 12);
    data |= ((long)bmeReadRegister(0xF8) << 8);
    data |= ((long)bmeReadRegister(0xF9) << 0);
    long var1, var2;
    unsigned long p;
    var1 = (((long)t_fine)>>1) - (long)64000;
    var2 = (((var1>>2) * (var1>>2)) >> 11 ) * ((long)dig.P6);
    var2 = var2 + ((var1*((long)dig.P5))<<1);
    var2 = (var2>>2)+(((long)dig.P4)<<16);
    var1 = (((dig.P3 * (((var1>>2) * (var1>>2)) >> 13 )) >> 3) + ((((long)dig.P2) * var1)>>1))>>18;
    var1 =((((32768+var1))*((long)dig.P1))>>15);
    if (var1 == 0)
       return 0; // avoid exception caused by division by zero
    p = (((unsigned long)(((long)1048576)-data)-(var2>>12)))*3125;
    if (p < 0x80000000)
        p = (p << 1) / ((unsigned long)var1);
    else
        p = (p / (unsigned long)var1) * 2;
    var1 = (((long)dig.P9) * ((long)(((p>>3) * (p>>3))>>13)))>>12;
    var2 = (((long)(p>>2)) * ((long)dig.P8))>>13;
    p = (unsigned long)((long)p + ((var1 + var2 + dig.P7) >> 4));
    return p/100;
}

#endif //bme_H_