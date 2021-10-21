/*
 * biosphere-monitor/main.c
 *
 * Created: 26.08.2021 21:30:14
 * Author : Tjark Gaudich
 * Target : BiosphereMonitor.brd
 * Fuses:
 * avrdude -c avrispmkII -p atxmega32a4u -U fuse1:w:0x00:m -U fuse2:w:0xdf:m -U fuse4:w:0xfe:m -U fuse5:w:0xff:m
 * 
 * Initialazation of a new Unit:
 * cc PC/biosphere.c -o PC/biosphere; ./PC/biosphere /dev/ttyUSB0 -delete -f -ct23 -i60 -t -r
 */ 

#ifndef __AVR_ATxmega32A4U__
#define __AVR_ATxmega32A4U__
#endif

#define F_CPU 16000000UL
#define BSCALE  -5
#define BSEL    246
#define ADCN    512       //Number of ADC readings taken per Messurment
#define ADRMAX  0x3FFFFF  //Highest Flash Adress
#define REDSIZE 16        //Size of a Reading in Flash

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <time.h>
#include <stdbool.h>
#include <avr/eeprom.h> 
#include <avr/pgmspace.h>
#include <stddef.h>
#include <avr/sleep.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "ATxmega_help.h"
#include "itoa.h"
#include "../reading.h"
#include "bme.h"
#include "Spi_Flash.h"

uint32_t timeCounter = 0;
volatile bool takeMessurment = false;
volatile bool instruct = false;
char uartBuf[16];
unsigned long f = 0;
unsigned long fOld= 0;
unsigned int  EEMEM intervall = 600;     //sampling intervall in Seconds
unsigned char EEMEM tOutOff = 0;         //Outside Temperature Offset in C*5 +128
unsigned char EEMEM tInOff = 0;          //Inside Temperature Offset in C*5 +128
unsigned long EEMEM flashAdr = 0;        //Adress of the start of the last reading in the SPI Flash

#define setIntervall(new)   eeprom_update_word(&intervall, new)
#define getIntervall        eeprom_read_word(&intervall)
#define settOutOff(new)     eeprom_update_byte(&tOutOff, new)
#define gettOutOff          (eeprom_read_byte(&tOutOff))
#define settInOff(new)      eeprom_update_byte(&tInOff, new)
#define gettInOff           (eeprom_read_byte(&tInOff))
#define setFlashAdr(new)    eeprom_update_dword(&flashAdr, new)
#define getFlashAdr         (eeprom_read_dword(&flashAdr))

struct reading getReading(void);
void printReading(struct reading in);
long selfDiagnosse(void);
void quicksort(unsigned int *data, unsigned int n);
unsigned int getMedian(unsigned int *rd, const unsigned int n);
unsigned char getOutsideTemp(void);
unsigned int getLight(void);
unsigned char getSoilHum(void);

int main(void)
{
    SET_CLK_EXTERN;
    PORTC.DIRSET = 0x08;
    RTC.PER = 3;
    RTC_INIT;

    PORTD.PIN5CTRL |= PORT_ISC_FALLING_gc;  //Frequency Counter initialization
    PORTD.INT0MASK |= (1 << 5);
    PORTD.INTCTRL = PORT_INT0LVL_LO_gc;

    ADC0_INIT;
    UART0INIT;
    bmeInit();
    sei();
    set_sleep_mode(SLEEP_MODE_EXT_STANDBY);
    Flash_init();
    _delay_ms(10);
    sleep_enable();

    while (1)
    {
        if(takeMessurment)
        {
            takeMessurment = false;
            PORTC.DIRSET = 0x08;
            struct reading in = getReading();

            unsigned long adrTmp = getFlashAdr;     //Jump to Adress of the new Reading
            adrTmp += REDSIZE;
            if (adrTmp > ADRMAX - REDSIZE)
                adrTmp = 0;
            setFlashAdr(adrTmp);

            if (adrTmp % 4096 == 0)    //Erase Sector if a new Sector is entert
                sectorErase4kB(adrTmp);

            byteWrite(in.timeRead >> 24,    adrTmp + 0);
            byteWrite(in.timeRead >> 16,    adrTmp + 1);
            byteWrite(in.timeRead >> 8,     adrTmp + 2);
            byteWrite(in.timeRead >> 0,     adrTmp + 3);
            byteWrite(in.light >> 8,        adrTmp + 4);
            byteWrite(in.light >> 0,        adrTmp + 5);
            byteWrite(in.temperaturOut >> 0,adrTmp + 6);
            byteWrite(in.temperaturIn >> 8, adrTmp + 7);
            byteWrite(in.temperaturIn >> 0, adrTmp + 8);
            byteWrite(in.pressure >> 8,     adrTmp + 9);
            byteWrite(in.pressure >> 0,     adrTmp + 10);
            byteWrite(in.humidityAir >> 0,  adrTmp + 11);
            byteWrite(in.humiditySoil >> 0, adrTmp + 12);
            byteWrite(in.iaq >> 8,          adrTmp + 13);
            byteWrite(in.iaq >> 0,          adrTmp + 14);
        }
        else if(instruct)
        {
            PORTC.DIRSET = 0x08;
            if(strncmp(uartBuf,"CR",2) == 0)
            {
                struct reading in = getReading();
                printReading(in);
            }
            else if(strncmp(uartBuf,"AR",2) == 0)
            {
                unsigned long adr = getFlashAdr;
                for(unsigned long i = 0; i <= adr; i += REDSIZE)
                {
                    struct reading in;
                    unsigned char tmp[REDSIZE];
                    READ(tmp, sizeof(tmp), i);

                    in.timeRead         = (long) tmp[0] << 24;
                    in.timeRead         |= (long) tmp[1] << 16;
                    in.timeRead         |= (long) tmp[2] << 8;
                    in.timeRead         |= (long) tmp[3] << 0;
                    in.light            = (int) tmp[4] << 8;
                    in.light            |= tmp[5];
                    in.temperaturOut    = tmp[6];
                    in.temperaturIn     = (int) tmp[7] << 8;
                    in.temperaturIn     |= tmp[8];
                    in.pressure         = (int) tmp[9] << 8;
                    in.pressure         |= tmp[10];
                    in.humidityAir      = tmp[11];
                    in.humiditySoil     = tmp[12];
                    in.iaq              = (int) tmp[13] << 8;
                    in.iaq              |= tmp[14];

                    printReading(in);
                }
                uartWriteString("EOF\r\n");
            }
            else if(strncmp(uartBuf,"DEL",3) == 0)
                setFlashAdr(ADRMAX);
            else if(strncmp(uartBuf,"OGT",3) == 0)
                uartWriteIntLine(gettOutOff);
            else if(strncmp(uartBuf,"OST",3) == 0)
                settOutOff(atoi(uartBuf+3));
            else if(strncmp(uartBuf,"OGI",3) == 0)
                uartWriteIntLine(gettInOff);
            else if(strncmp(uartBuf,"OSI",3) == 0)
                settInOff(atoi(uartBuf+3));
            else if(strncmp(uartBuf,"IG",2) == 0)
                uartWriteIntLine(getIntervall);
            else if(strncmp(uartBuf,"IS",2) == 0)
                setIntervall(atoi(uartBuf+2));
            else if(strncmp(uartBuf,"TG",2) == 0)
                uartWriteIntLine(timeCounter);
            else if(strncmp(uartBuf,"TS",2) == 0)
                timeCounter = atol(uartBuf+2);
            else if(strncmp(uartBuf,"DR",2) == 0)
                uartWriteIntLine(selfDiagnosse());
            else if(strncmp(uartBuf,"ID",2) == 0)
            {
                if(id > 0)
                    uartWriteIntLine(bmeReadRegister(0xD0));
                else
                    uartWriteIntLine(0);
            }
            instruct = 0;
        }
    }
}

struct reading getReading(void)     //reuturns fresh data
{
    struct reading in = {0,0,0,0,0,0,0,0};
    in.timeRead = timeCounter;
    in.temperaturOut = getOutsideTemp();
    in.light = getLight();
    in.humiditySoil = getSoilHum();
    if(id > 0) //BME installed
    {
        in.temperaturIn = getBmeTemp();
        in.pressure = getBmePress(in.temperaturOut*2);
    }
    if(id >=0x60)
        in.humidityAir = getBmeHumidity();
    if(id == 0x61)
        in.iaq = getBmeIaq();
    return in;
}

void printReading(struct reading in)    //prints in to UART
{
    char str[16];
    for (char i = 0; i < 8; i++)
    {
        _itoa(readingIt(&in,i), str);
        uartWriteString(str);
        if(i == 7)
            uartWriteString("\r\n");
        else
            uartWriteString(",");
    }
}

long selfDiagnosse(void)     //returns self diagnosis errorcode
{
    long errCode = 0;

    ADCA.CTRLA = ADC_ENABLE_bm;                     //Read Voltage at Vref Pin. Should always be 4095 counts
    ADCA.CH1.CTRL |= ADC_CH_START_bm;
    while(!(ADCA.CH1.INTFLAGS & ADC_CH_CHIF_bm));
    ADCA.CH1.INTFLAGS = ADC_CH_CHIF_bm;
    if(ADCA.CH1.RES < 4090)
        errCode |= (1 << 1);
    ADCA.CTRLA &= ~ADC_ENABLE_bm;

    time_t timeTmp = timeCounter;                   //is the RTC running?
    _delay_ms(1010);
    if(timeCounter - timeTmp < 1)
        errCode |= (1 << 2);

    if(timeCounter < 1577833200)                    //is the RTC initialized (time after 2020)?
        errCode |= (1 << 3);

    if(JEDEC_ID() != 0xBF258D)                      //Flash Signature as expected?
        errCode |= (1 << 4);

    if(flashAdr < (ADRMAX-4096))                    //Flash read/write check only if last sector isn't allready used
    {
        sectorErase4kB(ADRMAX-4096);
        byteWrite(0xAA, ADRMAX-4096);
        unsigned char test[1];
        READ(test, 1, ADRMAX-4096);
        if(test[0] == 0xFF)                            //Not Erased properly
            errCode |= (1 << 5);
        if(test[0] != 0xAA)                            //Read or Write went wrong
            errCode |= (1 << 6);
        sectorErase4kB(ADRMAX-4096);
    }

    if(!(PORTC.IN & (1 << 2)))                      //UART Rx and Tx should be high when nothing is tranmited
        errCode |= (1 << 7);
    if(!(PORTC.IN & (1 << 3)))
        errCode |= (1 << 8);

    unsigned char tTmp = getOutsideTemp();          //Temperatur Sensor Plausibility
    if(tTmp == 0 || tTmp > 250)
        errCode |= (1 << 9);

    if(fOld < 1100 || fOld > 10000)                 //Light Frequency Range
        errCode |= (1 << 10);

    if(getIntervall == UINT16_MAX)                  //Sampling Intervall not set
        errCode |= (1 << 11);

    if(gettOutOff == UINT8_MAX || gettInOff == UINT8_MAX)//Temperatur Offset not set
        errCode |= (1 << 12);

    if(id == 0)                                     //BME Connected?
        errCode |= (1 << 13);
    else                                            //BME Readings in range?
    {
        if(getBmeTemp() == 0 || getBmeTemp()  > 850 || getBmePress(tTmp*2) < 300 || getBmePress(tTmp*2)  > 1100)
            errCode |= (1 << 14);
    }
    return errCode;
}

void quicksort(unsigned int *data, unsigned int n)       //sorts data till n ascending
{
  if (n < 2) return;
 
  int pivot = data[n / 2];
 
  int i, j;
  for (i = 0, j = n - 1; ; i++, j--)
  {
    while (data[i] < pivot) i++;
    while (data[j] > pivot) j--;
 
    if (i >= j) break;
 
    int temp = data[i];
    data[i]     = data[j];
    data[j]     = temp;
  }
 
  quicksort(data, i);
  quicksort(data + i, n - i);
}

unsigned int getMedian(unsigned int *rd, const unsigned int n)   //returns Median of the Values in rd to rd[n]
{
    quicksort(rd, n);
	return n % 2 ? rd[n / 2] : (rd[n / 2 - 1] + rd[n / 2]) / 2;
}

unsigned char getOutsideTemp(void)  //returns Outside Temperatur in °C*5
{
    ADCA.CTRLA = ADC_ENABLE_bm;
    unsigned int tempArr[ADCN];
    for(int i = 0; i<ADCN; i++)
	{
		ADCA.CH0.CTRL |= ADC_CH_START_bm;
        while(!(ADCA.CH0.INTFLAGS & ADC_CH_CHIF_bm));
        ADCA.CH0.INTFLAGS = ADC_CH_CHIF_bm;
        tempArr[i] = ADCA.CH0.RES;
	}
    ADCA.CTRLA &= ~ADC_ENABLE_bm;
    return (getMedian(tempArr, ADCN)/8)+gettOutOff-128;
}

unsigned int getLight(void)  //returns iluminace in lux
{
    if(fOld < 1100)     //Sensor cutoff Frequency
        return 0;
    return (unsigned int) ((5.60669 * pow(2.3425, (fOld/1000.0)) ) + 141.444);
}

unsigned char getSoilHum(void)  //returns Soil Humidity in %
{
    ADCA.CTRLA = ADC_ENABLE_bm;
    unsigned int tempArr[ADCN];
    for(int i = 0; i<ADCN; i++)
	{
		ADCA.CH2.CTRL |= ADC_CH_START_bm;
        while(!(ADCA.CH2.INTFLAGS & ADC_CH_CHIF_bm));
        ADCA.CH2.INTFLAGS = ADC_CH_CHIF_bm;
        tempArr[i] = ADCA.CH2.RES;
	}
    ADCA.CTRLA &= ~ADC_ENABLE_bm;
    return (getMedian(tempArr, ADCN)*100/4096)-4;
}

ISR(USARTC0_RXC_vect)       //UART ISR
{
    uint8_t Data = USARTC0.DATA;
    USARTC0.DATA = Data;
    if(Data == '\r')
        USARTC0.DATA = '\n';
    static unsigned char i = 0;
    uartBuf[i] = Data;
    if(i == 15 || Data == '\r')
    {
        uartBuf[i] = '\0';
        instruct = true;
        i = 0;
    }
    else
        i++;
}

ISR(RTC_OVF_vect)          //RTC ISR
{
    sleep_disable();
    timeCounter++;
    fOld = f;
    f = 0;
    if(!(timeCounter % getIntervall))
        takeMessurment = true;
    else
        sleep_cpu();
}

ISR(PORTD_INT0_vect)
{
    f++;
}