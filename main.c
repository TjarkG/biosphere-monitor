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
#define F_CPU 32000000UL
#define BSCALE  -3
#define BSEL    1670
#define ADCN    32       //Number of ADC readings taken per Messurment

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <time.h>
#include <stdbool.h>
#include <avr/eeprom.h> 
#include "ATxmega_help.h"
#include "itoa.h"
#include "reading.h"

#define setBit(byte, bit, condition)	byte ^= (-(condition) ^ byte) & (1<<bit)

#define TestOut(bool)      if(bool) PORTD.OUT |= (1<<0); else PORTD.OUT &= ~(1<<0);
#define TestIn    (PORTD.IN & (1<<1)) 

time_t timeCounter = 0;
unsigned int EEMEM intervall = 600;         //sampling intervall in Seconds
bool         EEMEM soilSensor = false;      //is a Soil Sensor connected

#define setIntervall(new)   eeprom_update_word(&intervall, new)
#define getIntervall        eeprom_read_word(&intervall)
#define setSoilSensor(new)  eeprom_update_byte(&soilSensor, new)
#define hasSoilSensor       eeprom_read_byte(&soilSensor)

void linSort(unsigned short* data, unsigned char length)
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
    unsigned int tempArr[ADCN];
    for(int i = 0; i<ADCN; i++)
	{
		ADCA.CH0.CTRL |= ADC_CH_START_bm;
        while(!(ADCA.CH0.INTFLAGS & ADC_CH_CHIF_bm));
        ADCA.CH0.INTFLAGS = ADC_CH_CHIF_bm;
        tempArr[i] = ADCA.CH0.RES;
        _delay_ms(10);
	}
    //return getMedian(tempArr, ADCN);
    return (getMedian(tempArr, ADCN)*25)/32;
}

int main(void)
{
    SET_CLK_32MHZ
    PORTD.DIR = 0xFF;
    RTC.PER = 3;
    RTC_INIT;
    setIntervall(2);
    ADC0_INIT;
    UART0INIT; 
    sei();

    uartWriteString("Startup Completed\r\n");

    while (1)
    {
        TestOut(1);
        _delay_ms(100);
        TestOut(0);
        _delay_ms(100);
    }
}

ISR(USARTC0_RXC_vect)       //UART ISR
{
    uint8_t Data = USARTC0.DATA;
    USARTC0.DATA = Data;
    if(Data == '\r')
        USARTC0.DATA = '\n';
}

ISR(RTC_OVF_vect)          //RTC ISR
{
    timeCounter++;
    if(!(timeCounter % getIntervall))
    {
        char str[32];
        struct tm lt;
        (void) gmtime_r(&timeCounter, &lt);
        strftime(str, sizeof(str), "%d.%m.%Y %H:%M:%S ", &lt);
        uartWriteString(str);
        _itoa(getOutsideTemp(), str);
        uartWriteString(str);
        uartWriteString("\r\n");
    }
}