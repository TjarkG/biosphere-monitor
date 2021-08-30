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

unsigned char getOutsideTemp(void)
{
    ADCA.CH0.CTRL |= ADC_CH_START_bm;
    while(!(ADCA.CH0.INTFLAGS & ADC_CH_CHIF_bm));
    ADCA.CH0.INTFLAGS = ADC_CH_CHIF_bm;
    return ADCA.CH0.RES;
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
        _itoa(((unsigned long) getOutsideTemp()*1000)/255, str);
        uartWriteString(str);
        uartWriteString("\r\n");
    }
}