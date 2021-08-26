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

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "ATxmega_help.h"

#define setBit(byte, bit, condition)	byte ^= (-(condition) ^ byte) & (1<<bit)

#define TestOut(bool)      if(bool) PORTD.OUT |= (1<<0); else PORTD.OUT &= ~(1<<0);

#define TestIn    (PORTD.IN & (1<<1))

int main(void)
{
    SET_CLK_32MHZ
    PORTD.DIR = 0xFF;

    while (1)
    {
        TestOut(1);
        _delay_ms(100);
        TestOut(0);
        _delay_ms(100);
    }
}