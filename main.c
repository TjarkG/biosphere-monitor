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

#define setBit(byte, bit, condition)	byte ^= (-(condition) ^ byte) & (1<<bit)

#define TestOut(bool)      if(bool) PORTD.OUT |= (1<<0); else PORTD.OUT &= ~(1<<0);
#define TestIn    (PORTD.IN & (1<<1)) 

uint32_t timeCounter = 0;
volatile bool takeMessurment = false;
volatile bool instruct = false;
char uartBuf[16];
unsigned int  EEMEM intervall = 600;     //sampling intervall in Seconds
unsigned char EEMEM soilSensor = 0;      //is a Soil Sensor connected (bool)

#define setIntervall(new)   eeprom_update_word(&intervall, new)
#define getIntervall        eeprom_read_word(&intervall)
#define setSoilSensor(new)  eeprom_update_byte(&soilSensor, new)
#define hasSoilSensor       eeprom_read_byte(&soilSensor)

struct reading getReading(void);
void printReading(struct reading in);
int selfDiagnosse(void);

void linSort(unsigned int* data, unsigned char length)
{
	for (unsigned char i = 0; i < length; i++)
		for (unsigned char j = 0; j < length-i-1; j++)
			if (data[j] > data[j + 1])
			{
				short temp = data[j];
				data[j] = data[j + 1];
				data[j + 1] = temp;
			}
}

unsigned int getMedian(unsigned int *readings, const unsigned char n)
{
	linSort(readings, n);
	return n % 2 ? readings[n / 2] : (readings[n / 2 - 1] + readings[n / 2]) / 2;
}

unsigned char getOutsideTemp(void)
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

int main(void)
{
    SET_CLK_EXTERN;
    PORTD.DIRSET = 0x01;
    PORTC.DIRSET = 0x08;
    RTC.PER = 3;
    RTC_INIT;
    ADC0_INIT;
    UART0INIT;
    sei();
    set_sleep_mode(SLEEP_MODE_EXT_STANDBY);
    uartWriteString("BT\r\n");
    _delay_ms(10);
    sleep_enable();

    while (1)
    {
        if(!(PORTD.IN & (1<<1)))
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
            {
                char tmp[8];
                _itoa(getIntervall,tmp);
                uartWriteString(tmp);
                uartWriteString("\r\n");
            }
            else if(strncmp(uartBuf,"IS",2) == 0)
                setIntervall(atoi(uartBuf+2));
            else if(strncmp(uartBuf,"SG",2) == 0)
            {
                char tmp[8];
                _itoa(hasSoilSensor,tmp);
                uartWriteString(tmp);
                uartWriteString("\r\n");
            }
            else if(strncmp(uartBuf,"SS",2) == 0)
                setSoilSensor(atoi(uartBuf+2));
            else if(strncmp(uartBuf,"TG",2) == 0)
            {
                char tmp[16];
                _itoa(timeCounter,tmp);
                uartWriteString(tmp);
                uartWriteString("\r\n");
            }
            else if(strncmp(uartBuf,"TS",2) == 0)
                timeCounter = atol(uartBuf+2);
            else if(strncmp(uartBuf,"DR",2) == 0)
            {
                char tmp[12];
                _itoa(selfDiagnosse(),tmp);
                uartWriteString(tmp);
                uartWriteString("\r\n");
            }
            instruct = 0;
        }
    }
}

struct reading getReading(void)
{
    struct reading in = {0,0,0,0,0,0,0,0};
    in.timeRead = timeCounter;
    in.temperaturOut = getOutsideTemp();
    return in;
}

void printReading(struct reading in)
{
    char str[32];
    _itoa(in.timeRead, str);
    uartWriteString(str);
    uartWriteString(",");
    _itoa(in.light, str);
    uartWriteString(str);
    uartWriteString(",");
    _itoa(in.temperaturOut, str);
    uartWriteString(str);
    uartWriteString(",");
    _itoa(in.temperaturIn, str);
    uartWriteString(str);
    uartWriteString(",");
    _itoa(in.pressure, str);
    uartWriteString(str);
    uartWriteString(",");
    _itoa(in.humidityAir, str);
    uartWriteString(str);
    uartWriteString(",");
    _itoa(in.humiditySoil, str);
    uartWriteString(str);
    uartWriteString(",");
    _itoa(in.iaq, str);
    uartWriteString(str);
    uartWriteString("\r\n");
}

int selfDiagnosse(void)
{
    if(getOutsideTemp() == 0 || getOutsideTemp() > 100)
    {
        return -1;
    }
    return 0;
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