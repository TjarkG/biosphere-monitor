/*
 * Spi_Flash.h
 *
 * Header to communicate with a SST25VF040 Spi Flash on SPIC in Spi Mode 0
 * https://ww1.microchip.com/downloads/aemDocuments/documents/MPD/ProductDocuments/DataSheets/SST25VF040B-4-Mbit-SPI-Serial-Flash-Data-Sheet-20005051F.pdf
 *
 * Created: 25.09.2021 14:47:36
 *  Author: Tjark Gaudich
 */

#ifndef SPI_FLASH_H_
#define SPI_FLASH_H_

#include <avr/io.h>
#include <util/delay.h>

#define CE_LOW  PORTC.OUTCLR = (1 << 4)
#define CE_HIGH PORTC.OUTSET = (1 << 4)
#define TBP 12                                     //Wait Time after Write, in us
#define TSE 27                                     //Wait Time after Sector Erase, in ms

#define ADRMAX  0x3FFFFF                           //Highest Flash Adress

enum FlashInstruction
{
    WRSR = 0x01, PROG_BYTE, READ, WRDI, RDSR, WREN, 
    HS_READ = 0x0B, ERASE_4Kb = 0x20, EWSR = 0x50, ERASE_32Kb = 0x52, ERASE_64Kb = 0xD8,
    ERASE_CHIP = 0x60, EBSY = 0x70, DBSY = 0x80, RDID = 0x90, JEDEC_ID = 0x9F, PROG_WORD = 0xAD
} __attribute__ ((__packed__));

void flashInit(void);
uint8_t flashSpi(const uint8_t Data);
void flashSelectAddress(const uint8_t cmd, const uint32_t adress);
uint8_t flashStatus(void);
void writeEnable(void);
void chipErase(void);
inline uint32_t flashID(void);
void flashRead(uint8_t *out, const uint8_t n, const uint32_t adress);
void byteWrite(const uint8_t in, const uint32_t adress);
void flashWrite(uint8_t *in, const uint8_t size, const uint32_t adr);
void sectorErase4kB(const uint32_t adress);

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
    flashSpi(WRSR);
    flashSpi(0);
    CE_HIGH;
}

uint8_t flashSpi(const uint8_t Data)
{
    SPIC.DATA = Data;
    while(!(SPIC.STATUS & SPI_IF_bm));
    return SPIC.DATA;
}

void flashSelectAddress(const uint8_t cmd, const uint32_t adress)
{
    flashSpi(cmd);
    flashSpi(adress >> 16);
    flashSpi(adress >> 8);
    flashSpi(adress >> 0);
}

uint8_t flashStatus(void)    //Read Status Register
{
    CE_LOW;
    flashSpi(RDSR);
    uint8_t out = flashSpi(0);
    CE_HIGH;
    return out;
}

void writeEnable(void)
{
    CE_LOW;
    flashSpi(WREN);
    CE_HIGH;
}

void chipErase(void)
{
    writeEnable();
    CE_LOW;
    flashSpi(ERASE_CHIP);
    CE_HIGH;
}

inline uint32_t flashID(void)    //Read JEDEC-ID 
{
    CE_LOW;
    flashSpi(JEDEC_ID);
    uint32_t out;
    for(uint8_t i = 2; i >= 0 ; i--)
    {
        out |= (uint32_t) flashSpi(0) << (8*i);
    }
    CE_HIGH;
    return out;
}

void flashRead(uint8_t *out, const uint8_t n, const uint32_t adress)   //Read n Bytes from adress onwards
{
    CE_LOW;
    flashSelectAddress(READ, adress);
    for(uint8_t i = 0; i < n ; i++)
    {
        out[i] = flashSpi(0);
    }
    CE_HIGH;
}

void byteWrite(uint8_t in, const uint32_t adress)   //Write in to adress
{
    writeEnable();
    CE_LOW;
    flashSelectAddress(PROG_BYTE, adress);
    flashSpi(in);
    CE_HIGH;
    _delay_us(TBP);
}

void flashWrite(uint8_t *in, const uint8_t size, const uint32_t adr)
{
    for (uint8_t i = 0; i < size; i++)
    {
        byteWrite(in[i], adr+i);
    }
}

void sectorErase4kB(const uint32_t adress)   //Erases Sektor in whitch adress is lokated
{
    writeEnable();
    CE_LOW;
    flashSelectAddress(ERASE_4Kb, adress);
    CE_HIGH;
    _delay_ms(TSE);
}

#endif //SPI_FLASH_H_