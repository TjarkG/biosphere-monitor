/*
 * spiFlash.h
 *
 * Header to communicate with a SST25VF040 Spi Flash on SPIC in Spi Mode 0
 * https://ww1.microchip.com/downloads/aemDocuments/documents/MPD/ProductDocuments/DataSheets/SST25VF040B-4-Mbit-SPI-Serial-Flash-Data-Sheet-20005051F.pdf
 *
 * Created: 25.09.2021 14:47:36
 *  Author: Tjark Gaudich
 */

#ifndef SPI_FLASH_H_
#define SPI_FLASH_H_

#define ADR_MAX  0x3FFFFF                           //Highest Flash Address
#define SECTOR_SIZE 4096

enum FlashInstruction
{
    WRSR = 0x01, PROG_BYTE, READ, WRDI, RDSR, WREN, 
    HS_READ = 0x0B, ERASE_4Kb = 0x20, EWSR = 0x50, ERASE_32Kb = 0x52, ERASE_64Kb = 0xD8,
    ERASE_CHIP = 0x60, EBSY = 0x70, DBSY = 0x80, RD_ID = 0x90, JEDEC_ID = 0x9F, PROG_WORD = 0xAD
} __attribute__((unused));

void flashInit(void);
void flashChipErase(void);
uint32_t flashID(void);
void flashRead(void *out, uint8_t size, uint32_t adr);
void flashWrite(void *in, uint8_t size, uint32_t adr);
void flashErase4kB(uint32_t adr);

#endif //SPI_FLASH_H_