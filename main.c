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
#define BSCALE  -3
#define BSEL    835
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
#include "ATxmega_help.h"
#include "itoa.h"
#include "reading.h"

#define setBit(byte, bit, condition)	byte ^= (-(condition) ^ byte) & (1<<bit)

#define TestOut(bool)      if(bool) PORTD.OUT |= (1<<0); else PORTD.OUT &= ~(1<<0);
#define TestIn    (PORTD.IN & (1<<1)) 

time_t timeCounter = 0;
volatile bool takeMessurment = false;
unsigned int EEMEM intervall = 600;         //sampling intervall in Seconds
bool         EEMEM soilSensor = false;      //is a Soil Sensor connected

#define setIntervall(new)   eeprom_update_word(&intervall, new)
#define getIntervall        eeprom_read_word(&intervall)
#define setSoilSensor(new)  eeprom_update_byte(&soilSensor, new)
#define hasSoilSensor       eeprom_read_byte(&soilSensor)

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
    PORTD.DIRSET = 0xFF;
    PORTC.DIRSET = 0x08;
    RTC.PER = 3;
    RTC_INIT;
    setIntervall(4);
    ADC0_INIT;
    UART0INIT;
    sei();
    set_sleep_mode(SLEEP_MODE_EXT_STANDBY);
    uartWriteString("Startup Completed\r\n");
    _delay_ms(10);
    sleep_enable();

    while (1)
    {
        if(takeMessurment)
        {
            takeMessurment = false;
            char str[32];
            struct tm lt;
            (void) gmtime_r(&timeCounter, &lt);
            strftime(str, sizeof(str), "%d.%m.%Y %H:%M:%S ", &lt);
            uartWriteString(str);
            _itoa(getOutsideTemp(), str);
            uartWriteString(str);
            uartWriteString("\r\n");
            _delay_ms(10);
        }
        sleep_enable();
        sleep_cpu();
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
    sleep_disable();
    timeCounter++;
    if(!(timeCounter % getIntervall))
        takeMessurment = true;
    else
        sleep_cpu();
}