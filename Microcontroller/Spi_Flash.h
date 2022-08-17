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

enum FlashInstruction
{
    WRSR = 0x01, PROG_BYTE, READ, WRDI, RDSR, WREN, 
    HS_READ = 0x0B, ERASE_4Kb = 0x20, EWSR = 0x50, ERASE_32Kb = 0x52, ERASE_64Kb = 0xD8,
    ERASE_CHIP = 0x60, EBSY = 0x70, DBSY = 0x80, RDID = 0x90, JEDEC_ID = 0x9F, PROG_WORD = 0xAD
} __attribute__ ((__packed__));

inline void flashInit(void);
uint8_t flashSpi(uint8_t Data);
void flashSelectAddress(uint8_t cmd, uint32_t adress);
uint8_t flashStatus(void);
inline uint32_t flashID(void);
void flashRead(uint8_t *out, uint8_t n, uint32_t adress);
void byteWrite(uint8_t in, uint32_t adress);

inline void flashInit(void)
{
    PORTD.DIRSET = (1 << 0) | (1 << 1);
    PORTC.DIRSET = (1 << 4);
    CE_HIGH;

    SPIC.CTRL |= (SPI_ENABLE_bm | SPI_MASTER_bm | SPI_MODE_0_gc | SPI_PRESCALER_DIV4_gc | SPI_CLK2X_bm);
}

uint8_t flashSpi(uint8_t Data)
{
    SPIC.DATA = Data;
    while(!(SPIC.STATUS & SPI_IF_bm));
    return SPIC.DATA;
}

inline void flashSelectAddress(uint8_t cmd, uint32_t adress)
{
    adress |= ((uint32_t) cmd << 24);
    for(uint8_t i = 3; i >= 0 ; i--)
        flashSpi(adress << (8*i));
}

uint8_t flashStatus(void)    //Read Status Register
{
    CE_LOW;
    flashSpi(RDSR);
    uint8_t out = flashSpi(0);
    CE_HIGH;
    return out;
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

void flashRead(uint8_t *out, uint8_t n, uint32_t adress)   //Read n Bytes from adress onwards
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
    CE_LOW;
    flashSpi(WREN);
    flashSelectAddress(PROG_BYTE, adress);
    flashSpi(in);
    CE_HIGH;
    _delay_us(TBP);
}

void FlashWrite(uint8_t *in, const uint8_t size, const uint32_t adr)
{
    for (uint8_t i = 0; i < size; i++)
    {
        byteWrite(in[i], adr+i);
    }
}

void sectorErase4kB(uint32_t adress)   //Erases Sektor in which adress is located
{
    CE_LOW;
    flashSpi(WREN);
    flashSelectAddress(ERASE_4Kb, adress);
    CE_HIGH;
    _delay_ms(TSE);
}

#endif //SPI_FLASH_H_