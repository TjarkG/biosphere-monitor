//Writen by TjarkG and published under the MIT License

#define F_CPU 16000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <stdbool.h>
#include <string.h>
#include "bme.h"

enum Type id;      //Sensor ID

static long t_fine;     //Fine Temperatur for Pressur Compensation

static struct Calibration    //compensation Values
{
    short T[3];
    short P[10];
    char H[8];
    char GH[4];
} dig;

bool waitRx(void);
bool read(const uint8_t reg, uint8_t *buf, const uint8_t n);
uint32_t readADC(const uint8_t reg);
void write(const uint8_t reg, const uint8_t data);

bool waitRx(void)
{
    for (uint16_t i = 0; i < 1000; i++)
    {
        if(TWIC.MASTER.STATUS & TWI_MASTER_RIF_bm)
            return true;
        _delay_us(1);
    }

    return false;
}

#define WaitTx while(!(TWIC.MASTER.STATUS & TWI_MASTER_WIF_bm))

//read n bytes after reg into buf
bool read(const uint8_t reg, uint8_t *buf, const uint8_t n)
{
    //Select Register
    TWIC.MASTER.ADDR = 0x76 << 1;
    WaitTx;
    TWIC.MASTER.DATA = reg;
    WaitTx;
    TWIC.MASTER.CTRLC = TWI_MASTER_CMD_STOP_gc;

    //read
    TWIC.MASTER.ADDR = (0x76 << 1) | 0x01;
    if(!waitRx())
        return false;

    for (uint8_t i = 0; i < n; i++)
    {
        if(!waitRx())
            return false;
        buf[i] = TWIC.MASTER.DATA;
        if(n - i != 0)
            TWIC.MASTER.CTRLC = TWI_MASTER_CMD_RECVTRANS_gc;
    }
    
    TWIC.MASTER.CTRLC = TWI_MASTER_ACKACT_bm | TWI_MASTER_CMD_STOP_gc;
    return true;
}

uint32_t readADC(const uint8_t reg)
{
    uint8_t buf[3];
    read(reg, buf, sizeof buf);

    return (((uint32_t) buf[0] << 12) & 0xFF000) | (((uint32_t) buf[1] << 4) & 0x00FF0) | ((uint32_t) buf[2] >> 4);
}

void write(const uint8_t reg, const uint8_t data)
{
    //Select Register
    TWIC.MASTER.ADDR = 0x76 << 1;
    WaitTx;
    TWIC.MASTER.DATA = reg;
    WaitTx;

    //write data
    TWIC.MASTER.DATA = data;
    WaitTx;
    TWIC.MASTER.CTRLC = TWI_MASTER_CMD_STOP_gc;
}

void bmeInit(void)
{
    //TWI Setup
    TWIC.MASTER.BAUD = 155;
    TWIC.MASTER.CTRLA = TWI_MASTER_ENABLE_bm;
    TWIC.MASTER.STATUS |= TWI_MASTER_BUSSTATE_IDLE_gc;

    //Read Device ID with timeout (incase nothing is connected)
    if(!read(0xD0, &id, 1))
        return;

    if(id == BMP280 || id == BME280)
    {
        /*see 
         * https://www.bosch-sensortec.com/media/boschsensortec/downloads/datasheets/bst-bmp280-ds001.pdf p. 21 and
         * https://www.bosch-sensortec.com/media/boschsensortec/downloads/datasheets/bst-bme280-ds002.pdf p. 24 for reference
         */

        uint16_t buf[13];
        read(0x88, (uint8_t *) buf, sizeof buf);

        memcpy(dig.T, buf, 6);
        memcpy(dig.P, buf+3, 18);

        if(id == BME280)
        {
            uint8_t bufH[7];
            read(0xE1, bufH, sizeof bufH);

            dig.H[0] = ((uint8_t *) buf)[25]; 
            memcpy(dig.H+1, bufH, sizeof bufH);
        }
    }
    else if(id == BME680)
    {
        /*see 
         * https://www.bosch-sensortec.com/media/boschsensortec/downloads/datasheets/bst-bme680-ds001.pdf p. 18ff for reference
         */

        uint8_t bufE[16];
        read(0xE1, bufE, sizeof bufE);

        uint16_t buf8[12];
        read(0x8A, (uint8_t *)  buf8, sizeof buf8);

        dig.T[0] = bufE[8] | (bufE[9] << 8);
        dig.T[1] = buf8[0];
        dig.T[2] = ((uint8_t *) buf8)[2];

        memcpy(dig.P, buf8+2, 18); 
        dig.P[9] = ((uint8_t *) buf8)[22];

        memcpy(dig.H, bufE, 7);

        memcpy(dig.GH, bufE+10, 4);
        //GH1: dig.GH[2]
        //GH2: dig.GH[0] | dig.GH[1]
        //GH3: dig.GH[3]
    }
}

unsigned int getBmeTemp(void)  //returns BME Temperatur in °C*10
{
    if(id == BMP280 || id == BME280)
    {
        write(0xF4, (0x01 | (0b0011 << 2) | (0b0001 << 5)));
        _delay_ms(15);
        const int32_t data = readADC(0xFA);

        long var1, var2;
        var1  = ((((data>>3) - ((uint16_t) dig.T[0]<<1))) * ((long)dig.T[1])) >> 11;
        var2  = (((((data>>4) - ((uint16_t) dig.T[0])) * ((data>>4) - ((uint16_t) dig.T[0]))) >> 12) * ((long)dig.T[2])) >> 14;
        t_fine = var1 + var2;
    }
    else if(id == BME680)
    {
        write(0x74, (0x01 | (0b010 << 2) | (0b010 << 5)));
        _delay_ms(25);
        const int32_t data = readADC(0xFA);

        long var1, var2, var3;
        var1 = ((long)data >> 3) - ((long)dig.T[0] << 1); 
        var2 = (var1 * (long)dig.T[1]) >> 11; 
        var3 = ((((var1 >> 1) * (var1 >> 1)) >> 12) * ((long)dig.T[2] << 4)) >> 14; 
        t_fine = var2 + var3 - 3072;
    }
    return (t_fine/512);
}

unsigned int getBmePress(void)  //returns BME Pressure in hPa
{
    uint32_t p = 0;
    //No initialization needed, reading out data from Temperatur Messurment
    if(id == BMP280 || id == BME280)
    {
        const int32_t data = readADC(0xF7);

        int32_t var1, var2;
        var1 = (((int32_t)t_fine)>>1) - (int32_t)64000;
        var2 = (((var1>>2) * (var1>>2)) >> 11 ) * ((int32_t)dig.P[5]);
        var2 = var2 + ((var1*((int32_t)dig.P[4]))<<1);
        var2 = (var2>>2)+(((int32_t)dig.P[3])<<16);
        var1 = ((((int32_t)dig.P[2] * (((var1>>2) * (var1>>2)) >> 13 )) >> 3) + ((((int32_t)dig.P[1]) * var1)>>1))>>18;
        var1 =((((32768+var1))*(int32_t)((uint16_t)dig.P[0]))>>15);
        if (var1 == 0)
            return 0; // avoid exception caused by division by zero
        p = (((uint32_t)(((int32_t)1048576)-data)-(var2>>12)))*3125;
        if (p < 0x80000000)
            p = (p << 1) / ((uint32_t)var1);
        else
            p = (p / (uint32_t)var1) * 2;
        var1 = (((int32_t)dig.P[8]) * ((int32_t)(((p>>3) * (p>>3))>>13)))>>12;
        var2 = (((int32_t)(p>>2)) * ((int32_t)dig.P[7]))>>13;
        p = (uint32_t)((int32_t)p + ((var1 + var2 + (int32_t)dig.P[6]) >> 4));
    }
    else if(id == BME680)
    {
        const int32_t data = readADC(0x1F);
        long var1, var2, var3;
        var1 = ((long)t_fine >> 1) - 64000;  
        var2 = ((((var1 >> 2) * (var1 >> 2)) >> 11) * (long)(dig.P[5] && 0xFF)) >> 2;  
        var2 = var2 + ((var1 * (long)dig.P[4]) << 1);   
        var2 = (var2 >> 2) + ((long)dig.P[3] << 16);  
        var1 = (((((var1 >> 2) * (var1 >> 2)) >> 13) *((long)(dig.P[2] && 0xFF) << 5)) >> 3) + (((long)dig.P[1] * var1) >> 1); 
        var1 = var1 >> 18;  
        var1 = ((32768 + var1) * (unsigned short)dig.P[0]) >> 15;  
        p = 1048576 - data;  
        p = (unsigned long)((p - (var2 >> 12)) * ((unsigned long)3125));  
        if (p >= (1UL << 30))
            p = ((p / (unsigned long)var1) << 1);  
        else  
            p = ((p << 1) / (unsigned long)var1);  
        var1 = ((long)dig.P[8] * (long)(((p >> 3) * (p >> 3)) >> 13)) >> 12;  
        var2 = ((long)(p >> 2) * (long)dig.P[7]) >> 13;  
        var3 = ((long)(p >> 8) * (long)(p >> 8) * (long)(p >> 8) * (long)((unsigned char)dig.P[9])) >> 17;  
        p = (long)(p) + ((var1 + var2 + var3 + ((long)(dig.P[5] >> 8) << 7)) >> 4); 
    }
    return p/10;
}

unsigned char getBmeHumidity(void)
{
    if(id == BME280)
    {
        write(0xF2, 0b011);
        write(0xF4, (0x01 | (0b0011 << 2) | (0b0001 << 5)));
        _delay_ms(25);

        uint8_t buf[2];
        read(0xFD, buf, sizeof buf);
        const int16_t data = ((uint16_t) buf[0] << 8) | buf[1];

        double var; 
        var = (((double)t_fine) - 76800.0); 
        var = (data - (((double)((dig.H[5] & 0x0F) | ((uint16_t) dig.H[4]<<4))) * 64.0 + ((double)((dig.H[5] >> 4) | ((uint16_t) dig.H[6]<<4))) / 16384.0 * var)) *
        (((double)(((uint16_t) dig.H[2] << 8) | dig.H[1])) / 65536.0 * (1.0 + ((double)dig.H[7]) / 67108864.0 * var * (1.0 + ((double)dig.H[3]) / 67108864.0 * var))); 
        var = var * (1.0 - ((double)dig.H[0]) * var / 524288.0);
        
        if (var > 100.0) 
            var = 100.0; 
        else if (var < 0.0) 
            var = 0.0;
        return var;
    }
    else if(id == BME680)
    {
        write(0x72, 0b011);
        _delay_ms(25);

        uint8_t buf[2];
        read(0x25, buf, sizeof buf);
        const int16_t data = ((uint16_t) buf[0] << 8) | buf[1];

        long var1, var2, var3, var4, var5, var6;
        long temp_scaled = ((t_fine * 5) + 128) >> 8; 
        var1 = (long)data - (long)((long)((((dig.H[1]>>4 & 0xF) | dig.H[0]<<4) & 0xF) | dig.H[2]<<4) << 4) - 
        (((temp_scaled * (long)dig.H[2]) / ((long)100)) >> 1); 
        var2 = ((long)((dig.H[1]>>4 & 0xF) | dig.H[0]<<4) * (((temp_scaled * (long)dig.H[3]) / ((long)100)) + (((temp_scaled * ((temp_scaled * (long)dig.H[4]) / ((long)100))) >> 6) / ((long)100)) + ((long)(1 << 14)))) >> 10; 
        var3 = var1 * var2; 
        var4 = (((long)dig.H[5] << 7) + ((temp_scaled * (long)dig.H[6]) / ((long)100))) >> 4; 
        var5 = ((var3 >> 14) * (var3 >> 14)) >> 10; 
        var6 = (var4 * var5) >> 1;
        return ((((var3 + var6) >> 10) * ((long) 1000)) >> 12) / 1000; 
    }
    return 0;
}