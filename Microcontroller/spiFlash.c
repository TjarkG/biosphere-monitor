//
// Created by tjark on 25.03.23.
//

#include <avr/io.h>
#include <util/delay.h>
#include "spiFlash.h"

#define CE_LOW  PORTC.OUTCLR = (1 << 4)
#define CE_HIGH PORTC.OUTSET = (1 << 4)
#define TBP 12                                     //Wait Time after Write, in us
#define TSE 27                                     //Wait Time after Sector Erase, in ms

void selectAddress(uint8_t cmd, uint32_t adr);
void writeEnable(void);
void byteWrite(uint8_t in, uint32_t adr);
uint8_t spi(uint8_t Data);

void flashInit(void)
{
    PORTD.DIRSET = (1 << 0) | (1 << 1);
    PORTC.DIRSET = (1 << 4) | (1 << 5) | (1 << 7);
    PORTC.DIRCLR = (1 << 6);

    SPIC.CTRL |= (SPI_ENABLE_bm | SPI_MASTER_bm | SPI_MODE_0_gc | SPI_PRESCALER_DIV4_gc | SPI_CLK2X_bm);

    PORTD.OUTSET = (1 << 0) | (1 << 1);
    CE_HIGH;
    _delay_us(TBP);
    writeEnable();
    _delay_us(TBP);
    CE_LOW;
    spi(WRSR);
    spi(0);
    CE_HIGH;
}

uint8_t spi(uint8_t Data)
{
    SPIC.DATA = Data;
    while(!(SPIC.STATUS & SPI_IF_bm));
    return SPIC.DATA;
}

void selectAddress(uint8_t cmd, uint32_t adr)
{
    spi(cmd);
    spi(adr >> 16);
    spi(adr >> 8);
    spi(adr >> 0);
}

void writeEnable(void)
{
    CE_LOW;
    spi(WREN);
    CE_HIGH;
}

void flashChipErase(void)
{
    writeEnable();
    CE_LOW;
    spi(ERASE_CHIP);
    CE_HIGH;
}

inline uint32_t flashID(void)    //Read JEDEC-ID
{
    CE_LOW;
    spi(JEDEC_ID);
    uint32_t out = 0;
    out |= (uint32_t) spi(0) << 16;
    out |= (uint32_t) spi(0) << 8;
    out |= (uint32_t) spi(0);
    CE_HIGH;
    return out;
}

void flashRead(void *out, const uint8_t size, const uint32_t adr)   //Read size Bytes from adr onwards
{
    CE_LOW;
    selectAddress(READ, adr);
    for(uint8_t i = 0; i < size ; i++)
    {
        ((uint8_t *) out)[i] = spi(0);
    }
    CE_HIGH;
}

void byteWrite(uint8_t in, const uint32_t adr)   //Write in to adr
{
    writeEnable();
    CE_LOW;
    selectAddress(PROG_BYTE, adr);
    spi(in);
    CE_HIGH;
    _delay_us(TBP);
}

void flashWrite(void *in, const uint8_t size, const uint32_t adr)
{
    for (uint8_t i = 0; i < size; i++)
    {
        byteWrite(((uint8_t *) in)[i], adr+i);
    }
}

void flashErase4kB(uint32_t adr)   //Erases Sektor in which adr is located
{
    writeEnable();
    CE_LOW;
    selectAddress(ERASE_4Kb, adr);
    CE_HIGH;
    _delay_ms(TSE);
}