/*
 * Spi_Flash.h
 *
 * Header to communicate with a SST25VF049 Spi Flash on SPIC in Spi Mode 0
 *
 * Created: 25.09.2021 14:47:36
 *  Author: Tjark Gaudich
 */

#ifndef SPI_FLASH_H_
#define SPI_FLASH_H_

#ifndef __AVR_ATxmega32A4U__
#define __AVR_ATxmega32A4U__
#endif

#include <avr/io.h>
#include <util/delay.h>

#define CE_LOW  PORTC.OUTCLR = (1 << 4)
#define CE_HIGH PORTC.OUTSET = (1 << 4)
#define TBP 12                                     //Wait Time after Write, in us
#define TSE 27                                     //Wait Time after Sector Erase, in ms

void Flash_init(void);
unsigned char SPI_SendData(unsigned char Data);
unsigned char RDSR (void);
unsigned long JEDEC_ID (void);
void READ (unsigned char *out, unsigned char n, unsigned long adress);
void WREN (void);
void WRSR (void);
void byteWrite (unsigned char in, unsigned long adress);

void Flash_init(void)
{
    PORTD.DIRSET = (1 << 0) | (1 << 1);
    PORTC.DIRSET = (1 << 4) | (1 << 5) | (1 << 7);
    PORTC.DIRCLR = (1 << 6);

    SPIC.CTRL |= (SPI_ENABLE_bm | SPI_MASTER_bm | SPI_MODE_0_gc | SPI_PRESCALER_DIV64_gc);

    PORTD.OUTSET = (1 << 0) | (1 << 1);
    CE_HIGH;
    _delay_us(TBP);
    WREN();
    _delay_us(TBP);
    WRSR();
}

unsigned char SPI_SendData(unsigned char Data)
{
    SPIC.DATA = Data;
    while(!(SPIC.STATUS & SPI_IF_bm));
    return SPIC.DATA;
}

unsigned char RDSR (void)    //Read Status Register
{
    CE_LOW;
    SPI_SendData(0x05);
    unsigned char out = SPI_SendData(0);
    CE_HIGH;
    return out;
}

unsigned long JEDEC_ID (void)    //Read JEDEC-ID 
{
    CE_LOW;
    SPI_SendData(0x9F);
    unsigned long out = (long) SPI_SendData(0) << 16;
    out |= SPI_SendData(0) << 8;
    out |= SPI_SendData(0);
    CE_HIGH;
    return out;
}

void READ (unsigned char *out, unsigned char n, unsigned long adress)   //Read n Bytes from adress onwards
{
    CE_LOW;
    SPI_SendData(0x03);
    SPI_SendData(adress >> 16);
    SPI_SendData(adress >> 8);
    SPI_SendData(adress >> 0);
    for(unsigned char i = 0; i < n ; i++)
    {
        out[i] = SPI_SendData(0);
    }
    CE_HIGH;
}

void WREN (void)        //Write Enable
{
    CE_LOW;
    SPI_SendData(0x06);
    CE_HIGH;
}

void WRSR (void)        //Write Status Register
{
    CE_LOW;
    SPI_SendData(0x01);
    SPI_SendData(0x00);
    CE_HIGH;
}

void byteWrite (unsigned char in, unsigned long adress)   //Write in to adress
{
    WREN();
    CE_LOW;
    SPI_SendData(0x02);
    SPI_SendData(adress >> 16);
    SPI_SendData(adress >> 8);
    SPI_SendData(adress >> 0);
    SPI_SendData(in);
    CE_HIGH;
    _delay_us(TBP);
}

void chipErase(void)
{
    WREN();
    CE_LOW;
    SPI_SendData(0x60);
    CE_HIGH;
}

void sectorErase4kB(unsigned long adress)   //Erases Sektor in whitch adress is lokated
{
    WREN();
    CE_LOW;
    SPI_SendData(0x20);
    SPI_SendData(adress >> 16);
    SPI_SendData(adress >> 8);
    SPI_SendData(adress >> 0);
    CE_HIGH;
    _delay_ms(TSE);
}

#endif //SPI_FLASH_H_