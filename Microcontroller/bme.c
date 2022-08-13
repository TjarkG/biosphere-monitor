//Writen by TjarkG and published under the MIT License

#ifndef __AVR_ATxmega32A4U__
#define __AVR_ATxmega32A4U__
#endif

#define F_CPU 16000000UL
#define TARGTEMP 300    //Heater Temperature in °C

#include <avr/io.h>
#include <util/delay.h>

#define BMP280 0x58
#define BME280 0x60
#define BME680 0x61

static long t_fine;     //Fine Temperatur for Pressur Compensation
char id;                //Sensor ID

static struct cal    //compensation Values
{
    unsigned short T1;
    signed short T2;
    signed short T3;

    unsigned short P1;
    signed short P2;
    signed short P3;
    signed short P4;
    signed short P5;
    signed short P6;
    signed short P7;
    signed short P8;
    signed short P9;

    unsigned char P10;

    unsigned short H1;
    unsigned short H2;
    signed char H3;
    signed char H4;
    signed char H5;
    unsigned char H6;
    signed char H7;

    unsigned char HE1;
    signed short HE2;
    unsigned char HE3;
    signed short HE4;
    signed short HE5;
    signed char HE6;

    signed char GH1;
    signed short GH2;
    signed char GH3;
} dig;

char bmeInit(void);
unsigned char bmeReadRegister(const char reg);
long bmeRead20Bite(const char reg);
void bmeWriteRegister(const char reg, const unsigned char data);
void bmeSelectReg(const char reg);
unsigned int getBmeTemp(void);
unsigned int getBmePress(void);
unsigned char getBmeHumidity(void);
int getBmeIaq(void);

char bmeInit(void)
{
    TWIC.MASTER.BAUD = 155;
    TWIC.MASTER.CTRLA = TWI_MASTER_ENABLE_bm;
    TWIC.MASTER.STATUS |= TWI_MASTER_BUSSTATE_IDLE_gc;

    bmeSelectReg(0xD0);         //Get Device ID
    TWIC.MASTER.CTRLC = TWI_MASTER_CMD_STOP_gc;
    TWIC.MASTER.ADDR = (0x76 << 1) | 0x01;
    int i = 0;
    while(!(TWIC.MASTER.STATUS & TWI_MASTER_RIF_bm))
    {
        _delay_us(1);
        i++;
        if(i > 1000)
            return 0;
    }
    while(!(TWIC.MASTER.STATUS & TWI_MASTER_RIF_bm));
    id = TWIC.MASTER.DATA;
    TWIC.MASTER.CTRLC = TWI_MASTER_ACKACT_bm | TWI_MASTER_CMD_STOP_gc;

    if(id == BMP280 || id == BME280)
    {
        dig.T1 = bmeReadRegister(0x88);
        dig.T1 |= bmeReadRegister(0x89)<<8;
        dig.T2 = bmeReadRegister(0x8A);
        dig.T2 |= bmeReadRegister(0x8B)<<8;
        dig.T3 = bmeReadRegister(0x8C);
        dig.T3 |= bmeReadRegister(0x8D)<<8;

        dig.P1 = bmeReadRegister(0x8E);
        dig.P1 |= bmeReadRegister(0x8F)<<8;
        dig.P2 = bmeReadRegister(0x90);
        dig.P2 |= bmeReadRegister(0x91)<<8;
        dig.P3 = bmeReadRegister(0x92);
        dig.P3 |= bmeReadRegister(0x93)<<8;
        dig.P4 = bmeReadRegister(0x94);
        dig.P4 |= bmeReadRegister(0x95)<<8;
        dig.P5 = bmeReadRegister(0x96);
        dig.P5 |= bmeReadRegister(0x97)<<8;
        dig.P6 = bmeReadRegister(0x98);
        dig.P6 |= bmeReadRegister(0x99)<<8;
        dig.P7 = bmeReadRegister(0x9A);
        dig.P7 |= bmeReadRegister(0x9B)<<8;
        dig.P8 = bmeReadRegister(0x9C);
        dig.P8 |= bmeReadRegister(0x9D)<<8;
        dig.P9 = bmeReadRegister(0x9E);
        dig.P9 |= bmeReadRegister(0x9F)<<8;

        if(id == BME280)
        {
            dig.HE1 = bmeReadRegister(0xA1);
            dig.HE2 = bmeReadRegister(0xE1);
            dig.HE2 |= bmeReadRegister(0xE2)<<8;
            dig.HE3 = bmeReadRegister(0xE3);
            dig.HE4 = bmeReadRegister(0xE4)<<4;
            dig.HE4 |= (bmeReadRegister(0xE5) & 0x0F);
            dig.HE5 = (bmeReadRegister(0xE5) & 0xF0) >> 4;
            dig.HE5 |= bmeReadRegister(0xE6)<<4;
            dig.HE6 = bmeReadRegister(0xE7);
        }
    }
    else if(id == BME680)
    {
        dig.T1 = bmeReadRegister(0xE9);
        dig.T1 |= bmeReadRegister(0xEA)<<8;
        dig.T2 = bmeReadRegister(0x8A);
        dig.T2 |= bmeReadRegister(0x8B)<<8;
        dig.T3 = bmeReadRegister(0x8C);

        dig.P1 = bmeReadRegister(0x8E);
        dig.P1 |= bmeReadRegister(0x8F)<<8;
        dig.P2 = bmeReadRegister(0x90);
        dig.P2 |= bmeReadRegister(0x91)<<8;
        dig.P3 = bmeReadRegister(0x92);
        dig.P4 = bmeReadRegister(0x94);
        dig.P4 |= bmeReadRegister(0x95)<<8;
        dig.P5 = bmeReadRegister(0x96);
        dig.P5 |= bmeReadRegister(0x97)<<8;
        dig.P6 = bmeReadRegister(0x99);
        dig.P7 = bmeReadRegister(0x98);
        dig.P8 = bmeReadRegister(0x9C);
        dig.P8 |= bmeReadRegister(0x9D)<<8;
        dig.P9 = bmeReadRegister(0x9E);
        dig.P9 |= bmeReadRegister(0x9F)<<8;
        dig.P10 = bmeReadRegister(0xA0);

        dig.H1 = (bmeReadRegister(0xE2) & 0xF);
        dig.H1 |= bmeReadRegister(0xE3)<<4;
        dig.H2 = (bmeReadRegister(0xE2)>>4 & 0xF);
        dig.H2 |= bmeReadRegister(0xE1)<<4;
        dig.H3 = bmeReadRegister(0xE4);
        dig.H4 = bmeReadRegister(0xE5);
        dig.H5 = bmeReadRegister(0xE6);
        dig.H6 = bmeReadRegister(0xE7);
        dig.H7 = bmeReadRegister(0xE8);

        dig.GH1 = bmeReadRegister(0xED);
        dig.GH2 = bmeReadRegister(0xEB);
        dig.GH2 |= bmeReadRegister(0xEC)<<8;
        dig.GH3 = bmeReadRegister(0xEE);
    }

    return id;
}

unsigned char bmeReadRegister(const char reg)
{
    bmeSelectReg(reg);
    TWIC.MASTER.CTRLC = TWI_MASTER_CMD_STOP_gc;

    TWIC.MASTER.ADDR = (0x76 << 1) | 0x01;
    while(!((TWIC.MASTER.STATUS & TWI_MASTER_RIF_bm)));
    unsigned char data = 0;
    while(!(TWIC.MASTER.STATUS & TWI_MASTER_RIF_bm));
    data = TWIC.MASTER.DATA;
    TWIC.MASTER.CTRLC = TWI_MASTER_ACKACT_bm | TWI_MASTER_CMD_STOP_gc;
    return data;
}

long bmeRead20Bite(const char reg)
{
    long data = 0;
    bmeSelectReg(reg);
    TWIC.MASTER.CTRLC = TWI_MASTER_CMD_STOP_gc;

    TWIC.MASTER.ADDR = (0x76 << 1) | 0x01;
    while(!((TWIC.MASTER.STATUS & TWI_MASTER_RIF_bm)));

    while(!(TWIC.MASTER.STATUS & TWI_MASTER_RIF_bm));
    data |= ((unsigned long)TWIC.MASTER.DATA << 12) & 0xFF000;
    TWIC.MASTER.CTRLC = TWI_MASTER_CMD_RECVTRANS_gc;
    while(!(TWIC.MASTER.STATUS & TWI_MASTER_RIF_bm));
    data |= ((unsigned long)TWIC.MASTER.DATA << 4) & 0x00FF0;
    TWIC.MASTER.CTRLC = TWI_MASTER_CMD_RECVTRANS_gc;
    while(!(TWIC.MASTER.STATUS & TWI_MASTER_RIF_bm));
    data |= ((unsigned long)TWIC.MASTER.DATA >> 4) & 0x0000F;

    TWIC.MASTER.CTRLC = TWI_MASTER_ACKACT_bm | TWI_MASTER_CMD_STOP_gc;
    return data;
}

void bmeWriteRegister(const char reg, const unsigned char data)
{
    bmeSelectReg(reg);
    while(!(TWIC.MASTER.STATUS & TWI_MASTER_WIF_bm));
    TWIC.MASTER.DATA = data;
    while(!(TWIC.MASTER.STATUS & TWI_MASTER_WIF_bm));
    TWIC.MASTER.CTRLC = TWI_MASTER_CMD_STOP_gc;
}

void bmeSelectReg(const char reg)
{
    TWIC.MASTER.ADDR = 0x76 << 1;
    while(!((TWIC.MASTER.STATUS & TWI_MASTER_WIF_bm)));
    TWIC.MASTER.DATA = reg;
    while(!(TWIC.MASTER.STATUS & TWI_MASTER_WIF_bm));
}

unsigned int getBmeTemp(void)  //returns BME Temperatur in °C*10
{
    long data = 0;
    if(id == BMP280 || id == BME280)
    {
        bmeWriteRegister(0xF4, (0x01 | (0b0011 << 2) | (0b0001 << 5)));
        _delay_ms(15);
        data = bmeRead20Bite(0xFA);

        long var1, var2;
        var1  = ((((data>>3) - ((long)dig.T1<<1))) * ((long)dig.T2)) >> 11;
        var2  = (((((data>>4) - ((long)dig.T1)) * ((data>>4) - ((long)dig.T1))) >> 12) * ((long)dig.T3)) >> 14;
        t_fine = var1 + var2;
    }
    else if(id == BME680)
    {
        bmeWriteRegister(0x74, (0x01 | (0b010 << 2) | (0b010 << 5)));
        _delay_ms(25);
        data = bmeRead20Bite(0xFA);

        long var1, var2, var3;
        var1 = ((long)data >> 3) - ((long)dig.T1 << 1); 
        var2 = (var1 * (long)dig.T2) >> 11; 
        var3 = ((((var1 >> 1) * (var1 >> 1)) >> 12) * ((long)dig.T3 << 4)) >> 14; 
        t_fine = var2 + var3 - 3072;
    }
    return (t_fine/512);
}

unsigned int getBmePress(void)  //returns BME Pressure in hPa
{
    long data = 0;
    unsigned long p = 0;
    //No initialization needed, reading out data from Temperatur Messurment
    data = bmeRead20Bite(0xF7);
    if(id == BMP280 || id == BME280)
    {
        long var1, var2;
        var1 = (((long)t_fine)>>1) - (long)64000;
        var2 = (((var1>>2) * (var1>>2)) >> 11 ) * ((long)dig.P6);
        var2 = var2 + ((var1*((long)dig.P5))<<1);
        var2 = (var2>>2)+(((long)dig.P4)<<16);
        var1 = (((dig.P3 * (((var1>>2) * (var1>>2)) >> 13 )) >> 3) + ((((long)dig.P2) * var1)>>1))>>18;
        var1 =((((32768+var1))*((long)dig.P1))>>15);
        if (var1 == 0)
           return 0; // avoid exception caused by division by zero
        p = (((unsigned long)(((long)1048576)-data)-(var2>>12)))*3125;
        if (p < 0x80000000)
            p = (p << 1) / ((unsigned long)var1);
        else
            p = (p / (unsigned long)var1) * 2;
        var1 = (((long)dig.P9) * ((long)(((p>>3) * (p>>3))>>13)))>>12;
        var2 = (((long)(p>>2)) * ((long)dig.P8))>>13;
        p = (unsigned long)((long)p + ((var1 + var2 + dig.P7) >> 4));
    }
    else if(id == BME680)
    {
        long var1, var2, var3;
        var1 = ((long)t_fine >> 1) - 64000;  
        var2 = ((((var1 >> 2) * (var1 >> 2)) >> 11) * (long)dig.P6) >> 2;  
        var2 = var2 + ((var1 * (long)dig.P5) << 1);   
        var2 = (var2 >> 2) + ((long)dig.P4 << 16);  
        var1 = (((((var1 >> 2) * (var1 >> 2)) >> 13) *((long)dig.P3 << 5)) >> 3) + (((long)dig.P2 * var1) >> 1); 
        var1 = var1 >> 18;  
        var1 = ((32768 + var1) * (long)dig.P1) >> 15;  
        p = 1048576 - data;  
        p = (unsigned long)((p - (var2 >> 12)) * ((unsigned long)3125));  
        if (p >= (1UL << 30))  
            p = ((p / (unsigned long)var1) << 1);  
        else  
            p = ((p << 1) / (unsigned long)var1);  
        var1 = ((long)dig.P9 * (long)(((p >> 3) * (p >> 3)) >> 13)) >> 12;  
        var2 = ((long)(p >> 2) * (long)dig.P8) >> 13;  
        var3 = ((long)(p >> 8) * (long)(p >> 8) * (long)(p >> 8) * (long)dig.P10) >> 17;  
        p = (long)(p) + ((var1 + var2 + var3 + ((long)dig.P7 << 7)) >> 4); 
    }
    return p/100;
}

unsigned char getBmeHumidity(void)
{
    int data = 0;
    if(id == BME280)
    {
        bmeWriteRegister(0xF2, 0b011);
        bmeWriteRegister(0xF4, (0x01 | (0b0011 << 2) | (0b0001 << 5)));
        _delay_ms(25);
        data = ((unsigned int)bmeReadRegister(0xFD) << 8);
        data |= bmeReadRegister(0xFE);

        double var; 
        var = (((double)t_fine) - 76800.0); 
        var = (data - (((double)dig.HE4) * 64.0 + ((double)dig.HE5) / 16384.0 * var)) * (((double)dig.HE2) / 65536.0 * (1.0 + ((double)dig.HE6) /
        67108864.0 * var * (1.0 + ((double)dig.HE3) / 67108864.0 * var))); 
        var = var * (1.0 - ((double)dig.HE1) * var / 524288.0); 
        
        if (var > 100.0) 
            var = 100.0; 
        else if (var < 0.0) 
            var = 0.0;
        return var;
    }
    else if(id == BME680)
    {
        bmeWriteRegister(0x72, 0b011);
        _delay_ms(25);
        data = ((unsigned int)bmeReadRegister(0x25) << 8);
        data |= bmeReadRegister(0x26);

        long var1, var2, var3, var4, var5, var6;
        long temp_scaled = ((t_fine * 5) + 128) >> 8; 
        var1 = (long)data - (long)((long)dig.H1 << 4) - (((temp_scaled * (long)dig.H3) / ((long)100)) >> 1); 
        var2 = ((long)dig.H2 * (((temp_scaled * (long)dig.H4) / ((long)100)) + (((temp_scaled * ((temp_scaled * (long)dig.H5) / ((long)100))) >> 6) / ((long)100)) + ((long)(1 << 14)))) >> 10; 
        var3 = var1 * var2; 
        var4 = (((long)dig.H6 << 7) + ((temp_scaled * (long)dig.H7) / ((long)100))) >> 4; 
        var5 = ((var3 >> 14) * (var3 >> 14)) >> 10; 
        var6 = (var4 * var5) >> 1;
        return ((((var3 + var6) >> 10) * ((long) 1000)) >> 12) / 1000; 
    }
    return 0;
}