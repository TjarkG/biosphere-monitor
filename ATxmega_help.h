/*
 * ATxmega_help.h
 *
 * Header to help configuring xMega controllers
 *
 * Created: 04.08.2021 16:39:57
 *  Author: Tjark Gaudichs
 */

#ifndef ATxmega_help_H_
#define ATxmega_help_H_

#include <avr/io.h>

#define SET_CLK_32MHZ            \
    CCP = 0xD8;                  \
    OSC.CTRL = 0x02;             \
    while (!(OSC.STATUS & 0x02)) \
        ;                        \
    CCP = 0xD8;                  \
    CLK.CTRL = 0x01;

#endif /* ATxmega_help_H_ */