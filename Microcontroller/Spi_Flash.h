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

#define ADR_MAX  0x3FFFFF                           //Highest Flash Address

enum FlashInstruction
{
    WRSR = 0x01, PROG_BYTE, READ, WRDI, RDSR, WREN, 
    HS_READ = 0x0B, ERASE_4Kb = 0x20, EWSR = 0x50, ERASE_32Kb = 0x52, ERASE_64Kb = 0xD8,
    ERASE_CHIP = 0x60, EBSY = 0x70, DBSY = 0x80, RD_ID = 0x90, JEDEC_ID = 0x9F, PROG_WORD = 0xAD
} __attribute__((unused));

void flashInit(void);
uint8_t flashSpi(uint8_t Data);
void flashSelectAddress(uint8_t cmd, uint32_t address);
void writeEnable(void);
void chipErase(void);
inline uint32_t flashID(void);
void flashRead(uint8_t *out, uint8_t n, uint32_t address);
void byteWrite(uint8_t in, uint32_t address);
void flashWrite(uint8_t *in, uint8_t size, uint32_t adr);
void sectorErase4kB(uint32_t address);

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

void flashSelectAddress(const uint8_t cmd, const uint32_t address)
{
    flashSpi(cmd);
    flashSpi(address >> 16);
    flashSpi(address >> 8);
    flashSpi(address >> 0);
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
    uint32_t out = 0;
    for(uint8_t i = 2; i >= 0 ; i--)
    {
        out |= (uint32_t) flashSpi(0) << (8*i);
    }
    CE_HIGH;
    return out;
}

void flashRead(uint8_t *out, const uint8_t n, const uint32_t address)   //Read n Bytes from address onwards
{
    CE_LOW;
    flashSelectAddress(READ, address);
    for(uint8_t i = 0; i < n ; i++)
    {
        out[i] = flashSpi(0);
    }
    CE_HIGH;
}

void byteWrite(uint8_t in, const uint32_t address)   //Write in to address
{
    writeEnable();
    CE_LOW;
    flashSelectAddress(PROG_BYTE, address);
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

void sectorErase4kB(const uint32_t address)   //Erases Sektor in which address is located
{
    writeEnable();
    CE_LOW;
    flashSelectAddress(ERASE_4Kb, address);
    CE_HIGH;
    _delay_ms(TSE);
}

#endif //SPI_FLASH_H_