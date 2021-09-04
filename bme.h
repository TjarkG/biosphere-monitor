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
unsigned short getBmeTemp(void);
void bmeSelectReg(const char reg);

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

unsigned short getBmeTemp(void)  //returns Inside Temperatur in °C*10
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

#endif //bme_H_