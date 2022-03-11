/*
 * ATxmega_help.h
 *
 * Header to help configuring xMega controllers
 *
 * Created: 04.08.2021 16:39:57
 *  Author: Tjark Gaudich
 */

#ifndef ATxmegaAux_H_
#define ATxmegaAux_H_

#ifndef __AVR_ATxmega32A4U__
#define __AVR_ATxmega32A4U__
#endif

#include <avr/io.h>
#include "itoa.h"

#define SET_CLK_32MHZ            \
    CCP = 0xD8;                  \
    OSC.CTRL = 0x02;             \
    while (!(OSC.STATUS & 0x02)) \
        ;                        \
    CCP = 0xD8;                  \
    CLK.CTRL = 0x01

#define SET_CLK_EXTERN                                                          \
    OSC_XOSCCTRL = OSC_XOSCSEL_XTAL_16KCLK_gc | OSC_FRQRANGE_12TO16_gc;\
    OSC.CTRL |= OSC_XOSCEN_bm;\
    while(!(OSC.STATUS & OSC_XOSCRDY_bm));\
    CCP = CCP_IOREG_gc;\
    CLK.CTRL = CLK_SCLKSEL_XOSC_gc

#define UART0INIT                                                               \
    USARTC0.BAUDCTRLA = (BSEL & 0xFF);                                          \
    USARTC0.BAUDCTRLB = ((BSEL & 0xF00) >> 8);                                  \
    USARTC0.BAUDCTRLB |= ((BSCALE & 0x0F) << 4);                                \
    USARTC0.CTRLA = USART_RXCINTLVL_LO_gc;                                      \
    USARTC0.STATUS |= USART_RXCIF_bm;                                           \
    USARTC0.CTRLB = USART_TXEN_bm | USART_RXEN_bm;                              \
    USARTC0.CTRLC = USART_CHSIZE_8BIT_gc;                                       \
    USARTC0.CTRLC &= ~(USART_PMODE0_bm | USART_PMODE1_bm | USART_SBMODE_bm);    \
    PMIC.CTRL = PMIC_LOLVLEN_bm

#define RTC_INIT                                                                \
    OSC.XOSCCTRL = OSC_X32KLPM_bm | OSC_XOSCSEL_32KHz_gc;                       \
    OSC.CTRL = OSC_XOSCEN_bm;                                                   \
    while(!(OSC.STATUS & OSC_XOSCRDY_bm));                                      \
    CLK.RTCCTRL = CLK_RTCSRC_TOSC_gc | CLK_RTCEN_bm;                            \
    RTC.CTRL = RTC_PRESCALER_DIV256_gc;                                         \
    while(OSC.STATUS & RTC_SYNCBUSY_bm);                                        \
    RTC.INTCTRL = RTC_OVFINTLVL_LO_gc;                                          \
    PMIC.CTRL = PMIC_LOLVLEN_bm

#define ADC0_INIT                                                               \
    ADCA.CTRLB = ADC_RESOLUTION_12BIT_gc;                                       \
    ADCA.REFCTRL = ADC_REFSEL_AREFA_gc;                                         \
    ADCA.PRESCALER = ADC_PRESCALER_DIV32_gc;                                    \
    ADCA.CTRLA = ADC_ENABLE_bm;                                                 \
    ADCA.CH0.CTRL = ADC_CH_INPUTMODE0_bm;                                       \
    ADCA.CH0.MUXCTRL = ADC_CH_MUXPOS_PIN3_gc;                                   \
    ADCA.CH1.CTRL = ADC_CH_INPUTMODE0_bm;                                       \
    ADCA.CH1.MUXCTRL = ADC_CH_MUXPOS_PIN0_gc;                                   \
    ADCA.CH2.CTRL = ADC_CH_INPUTMODE0_bm;                                       \
    ADCA.CH2.MUXCTRL = ADC_CH_MUXPOS_PIN2_gc

void uartWriteString(const char *in)
{
    while(*in)
    {
        while(!(USARTC0.STATUS & USART_DREIF_bm));
        USARTC0.DATA = *in++;
    }
}

void uartWriteIntLine(unsigned long in)
{
    _delay_ms(100);
    char tmp[12];
    _itoa(in,tmp);
    uartWriteString(tmp);
    uartWriteString("\r\n");
}

#endif /* ATxmegaAux_H_ */