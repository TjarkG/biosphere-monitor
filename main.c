/*
 * biosphere-monitor/main.c
 *
 * Created: 26.08.2021 21:30:14
 * Author : Tjark Gaudich
 * Target : BiosphereMonitor.brd
 */ 

#ifndef __AVR_ATxmega16A4U__
#define __AVR_ATxmega16A4U__
#endif

#define F_CPU 16000000UL
#define BSCALE  -5
#define BSEL    246
#define ADCN    32       //Number of ADC readings taken per Messurment

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
#include "reading.h"
#include "bme.h"

uint32_t timeCounter = 0;
volatile bool takeMessurment = false;
volatile bool instruct = false;
char uartBuf[16];
char sensType;
unsigned int  EEMEM intervall = 600;     //sampling intervall in Seconds
unsigned char EEMEM soilSensor = 0;      //is a Soil Sensor connected (bool)

#define setIntervall(new)   eeprom_update_word(&intervall, new)
#define getIntervall        eeprom_read_word(&intervall)
#define setSoilSensor(new)  eeprom_update_byte(&soilSensor, new)
#define hasSoilSensor       eeprom_read_byte(&soilSensor)

struct reading getReading(void);
void printReading(struct reading in);
int selfDiagnosse(void);
void linSort(unsigned int* data, unsigned char n);
unsigned int getMedian(unsigned int *rd, const unsigned char n);
unsigned char getOutsideTemp(void);
void uartWriteIntLine(long in);

int main(void)
{
    SET_CLK_EXTERN;
    PORTC.DIRSET = 0x08;
    RTC.PER = 3;
    RTC_INIT;
    ADC0_INIT;
    UART0INIT;
    sensType = bmeInit();
    sei();
    set_sleep_mode(SLEEP_MODE_EXT_STANDBY);
    uartWriteString("BT\r\n");
    _delay_ms(10);
    sleep_enable();

    while (1)
    {
        if(!(PORTD.IN & (1<<0)))
        {
            //PORTC.DIRCLR = 0x08;
            //sleep_enable();
            //sleep_cpu();
        }
        else if(takeMessurment)
        {
            takeMessurment = false;
            PORTC.DIRSET = 0x08;
            struct reading in = getReading();
            //printReading(in);
            in.iaq++;
            //TODO: Store Reading on SD Card
            _delay_ms(10);
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
                //TODO:print readings
                for(int i = 0; i<10; i++)
                {
                    struct reading in = getReading();
                    printReading(in);
                }
                uartWriteString("EOF\r\n");
            }
            else if(strncmp(uartBuf,"IG",2) == 0)
                uartWriteIntLine(getIntervall);
            else if(strncmp(uartBuf,"IS",2) == 0)
                setIntervall(atoi(uartBuf+2));
            else if(strncmp(uartBuf,"SG",2) == 0)
                uartWriteIntLine(hasSoilSensor);
            else if(strncmp(uartBuf,"SS",2) == 0)
                setSoilSensor(atoi(uartBuf+2));
            else if(strncmp(uartBuf,"TG",2) == 0)
                uartWriteIntLine(timeCounter);
            else if(strncmp(uartBuf,"TS",2) == 0)
                timeCounter = atol(uartBuf+2);
            else if(strncmp(uartBuf,"DR",2) == 0)
                uartWriteIntLine(selfDiagnosse());
            instruct = 0;
        }
    }
}

struct reading getReading(void)     //reuturns fresh data
{
    struct reading in = {0,0,0,0,0,0,0,0};
    in.timeRead = timeCounter;
    in.temperaturOut = getOutsideTemp();
    if(sensType > 0)
        in.temperaturIn = getBmeTemp();
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

void uartWriteIntLine(long in)
{
    char tmp[12];
    _itoa(in,tmp);
    uartWriteString(tmp);
    uartWriteString("\r\n");
}

int selfDiagnosse(void)     //returns self diagnosis errorcode
{
    if(getOutsideTemp() == 0 || getOutsideTemp() > 100)
    {
        return -1;
    }
    return 0;
}

void linSort(unsigned int* data, unsigned char n)       //sorts data till n ascending
{
	for (unsigned char i = 0; i < n; i++)
		for (unsigned char j = 0; j < n-i-1; j++)
			if (data[j] > data[j + 1])
			{
				short temp = data[j];
				data[j] = data[j + 1];
				data[j + 1] = temp;
			}
}

unsigned int getMedian(unsigned int *rd, const unsigned char n)   //returns Median of the Values in rd to rd[n]
{
	linSort(rd, n);
	return n % 2 ? rd[n / 2] : (rd[n / 2 - 1] + rd[n / 2]) / 2;
}

unsigned char getOutsideTemp(void)  //returns Outside Temperatur in °C*2
{
    ADCA.CTRLA = ADC_ENABLE_bm;
    _delay_ms(2);
    unsigned int tempArr[ADCN];
    for(int i = 0; i<ADCN; i++)
	{
		ADCA.CH0.CTRL |= ADC_CH_START_bm;
        while(!(ADCA.CH0.INTFLAGS & ADC_CH_CHIF_bm));
        ADCA.CH0.INTFLAGS = ADC_CH_CHIF_bm;
        tempArr[i] = ADCA.CH0.RES;
        _delay_ms(2);
	}
    ADCA.CTRLA &= ~ADC_ENABLE_bm;
    return (getMedian(tempArr, ADCN)*25)/32;
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
    if(!(timeCounter % getIntervall))
        takeMessurment = true;
    else
        sleep_cpu();
}