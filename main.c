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
#include "ATxmega_help.h"
#include "itoa.h"

#define setBit(byte, bit, condition)	byte ^= (-(condition) ^ byte) & (1<<bit)

#define TestOut(bool)      if(bool) PORTD.OUT |= (1<<0); else PORTD.OUT &= ~(1<<0);
#define TestIn    (PORTD.IN & (1<<1))

time_t timeCounter = 0;

void toTimestamp(char* out, uint64_t time)
{

}

int main(void)
{
    SET_CLK_32MHZ
    PORTD.DIR = 0xFF;
    RTC_INIT;
    PORTE_REMAP;
    UART0INIT; 
    sei();

    uartWriteString("Startup Completed\r\n");

    while (1)
    {
        _delay_ms(500);
        TestOut(1);
        _delay_ms(100);
        TestOut(0);
        _delay_ms(100);
    }
}

ISR(USARTC0_RXC_vect)
{
    uint8_t Data = USARTC0.DATA;
    USARTC0.DATA = Data;
    if(Data == '\r')
        USARTC0.DATA = '\n';
}

ISR(RTC_OVF_vect)
{
    timeCounter++;
    char str[32];
    struct tm lt;
    (void) gmtime_r(&timeCounter, &lt);
    strftime(str, sizeof(str), "%d.%m.%Y %H:%M:%S", &lt);
    uartWriteString(str);
    uartWriteString("\r\n");
}