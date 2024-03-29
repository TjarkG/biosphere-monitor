/*
 * biosphere-monitor/main.c
 *
 * Created: 26.08.2021 21:30:14
 * Author : Tjark Gaudich
 * Target : BiosphereMonitor.brd
 * Fuses:
 * avrdude -c avrispmkII -p atxmega32a4u -U fuse1:w:0x00:m -U fuse2:w:0xdf:m -U fuse4:w:0xfe:m -U fuse5:w:0xff:m
 * 
 * Initialization of a new Unit:
 * make biosphere && ./PC/biosphere /dev/ttyUSB0 -delete -f -ct23 -i60 -t -r
 */ 

#define BSCALE  0        //Baudrate: 1000000
#define BSEL    0
#define ADC_N   512      //Number of ADC readings taken per measurement

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <util/delay.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include "../reading.h"
#include "ATxmegaAux.h"
#include "bme.h"
#include "spiFlash.h"

time_t timeCounter = 0;
volatile bool takeMeasurement = false;
volatile bool instruct = false;
char uartBuf[16];
uint16_t lightF = 0;
uint16_t lightFNom= 0;               //Frequency Counter Frequency in Hz

uint16_t EEMEM intervall = 600;     //sampling intervall in Seconds
uint8_t  EEMEM tOutOff = 0;         //Outside Temperature Offset in C*5 +128
uint32_t EEMEM flashAdr = 0;        //Address of the start of the last reading in the SPI Flash

uint16_t EEMEM lightOnTime = 0;     //light on Time
uint32_t EEMEM lightOffTime = 0;    //light off Time
uint32_t EEMEM lightThreshold = 0;   //Threshold under which light goes on (0 is always on)

#define setIntervall(new)   eeprom_update_word(&intervall, new)
#define getIntervall        eeprom_read_word(&intervall)
#define setTOutOff(new)     eeprom_update_byte(&tOutOff, new)
#define getTOutOff          eeprom_read_byte(&tOutOff)
#define setFlashAdr(new)    eeprom_update_dword(&flashAdr, new)
#define getFlashAdr         eeprom_read_dword(&flashAdr)

#define setLightOnTime(in)      eeprom_update_word(&lightOnTime, in)
#define getLightOnTime          eeprom_read_word(&lightOnTime)
#define setLightOffTime(in)     eeprom_update_dword(&lightOffTime, in)
#define getLightOffTime         eeprom_read_dword(&lightOffTime)
#define setLightThreshold(in)   eeprom_update_dword(&lightThreshold, in)
#define getLightThreshold       eeprom_read_dword(&lightThreshold)

struct reading getReading(void);
void printReading(struct reading in);
long selfDiagnosis(void);
void quicksort(uint16_t *data, uint16_t n);
uint16_t getMedian(uint16_t *rd, uint16_t n);
uint8_t getOutsideTemp(void);
uint16_t getLight(void);
uint16_t getExtCount(void);
uint8_t getRH(void);
uint32_t readingIt(struct reading *v, uint8_t i);

int main(void)
{
    clockExtern();
    PORTC.DIRSET = 0x08;
    PORTD.DIRSET = (1 << 3);
    RTC.PER = 3;
    rtc_init();

    PORTD.PIN5CTRL |= PORT_ISC_FALLING_gc;  //Frequency Counter initialization
    PORTD.INT0MASK |= (1 << 5);
    PORTD.INTCTRL = PORT_INT0LVL_LO_gc;

    ADCA_init();
    UART0_init();
    bmeInit();
    sei();
    flashInit();

    getReading();

    while (1)
    {
        if(takeMeasurement)
        {
            takeMeasurement = false;
            struct reading in = getReading();

            unsigned long adrTmp = getFlashAdr + sizeof(struct reading);     //Address of the new Reading
            if (adrTmp > (ADR_MAX - sizeof(struct reading)))
                adrTmp = 0;

            setFlashAdr(adrTmp);

            if (adrTmp % SECTOR_SIZE == 0)                 //Erase Sector if a new Sector is entert
                flashErase4kB(adrTmp);

            flashWrite(&in, sizeof(struct reading), adrTmp);
        }
        else if(instruct)
        {
            if (strncmp(uartBuf, "CR", 2) == 0)
                printReading(getReading());

            else if (strncmp(uartBuf, "AR", 2) == 0)
            {
                unsigned long adr = getFlashAdr;
                if (adr < ADR_MAX)
                    for (unsigned long i = 0; i <= adr; i += sizeof(struct reading))
                    {
                        struct reading in;
                        flashRead(&in, sizeof(struct reading), i);
                        printReading(in);
                    }
                uartWriteString("EOF\r\n");
            } else if (strncmp(uartBuf, "DEL", 3) == 0)
            {
                setFlashAdr(ADR_MAX);
                flashChipErase();
            } else if (strncmp(uartBuf, "OGT", 3) == 0)
                uartWriteIntLine(getTOutOff);
            else if (strncmp(uartBuf, "OST", 3) == 0)
                setTOutOff(strtol(uartBuf + 3, NULL, 10));
            else if (strncmp(uartBuf, "IG", 2) == 0)
                uartWriteIntLine(getIntervall);
            else if (strncmp(uartBuf, "IS", 2) == 0)
                setIntervall(strtol(uartBuf + 2, NULL, 10));
            else if (strncmp(uartBuf, "TG", 2) == 0)
                uartWriteIntLine(timeCounter);
            else if (strncmp(uartBuf, "TS", 2) == 0)
                timeCounter = strtol(uartBuf + 2, NULL, 10);
            else if (strncmp(uartBuf, "DR", 2) == 0)
                uartWriteIntLine(selfDiagnosis());
            else if (strncmp(uartBuf, "ID", 2) == 0)
                uartWriteIntLine(id);
            else if (strncmp(uartBuf, "RN", 2) == 0)
                uartWriteIntLine(getFlashAdr/sizeof(struct reading));
            else if (strncmp(uartBuf, "GH", 2) == 0)
                uartWriteIntLine(getExtCount());
            else if (strncmp(uartBuf, "GLN", 3) == 0)
                uartWriteIntLine(getLightOnTime);
            else if (strncmp(uartBuf, "SLN", 3) == 0)
                setLightOnTime(strtol(uartBuf + 3, NULL, 10));
            else if (strncmp(uartBuf, "GLF", 3) == 0)
                uartWriteIntLine(getLightOffTime);
            else if (strncmp(uartBuf, "SLF", 3) == 0)
                setLightOffTime(strtol(uartBuf + 3, NULL, 10));
            else if (strncmp(uartBuf, "GLT", 3) == 0)
                uartWriteIntLine(getLightThreshold);
            else if (strncmp(uartBuf, "SLT", 3) == 0)
                setLightThreshold(strtol(uartBuf + 3, NULL, 10));
            instruct = 0;
        }

        //check light conditions
        if((getLightOnTime < getLightOffTime ?
            (getLightOnTime < timeCounter%86400 && timeCounter%86400 < getLightOffTime) :
            (getLightOnTime < timeCounter%86400 || timeCounter%86400 < getLightOffTime))
            && (getLightThreshold == 0 || getLight() < getLightThreshold))
            PORTD.OUTSET = (1 << 3);
        else
            PORTD.OUTCLR = (1 << 3);
    }
}

struct reading getReading(void)     //returns fresh data
{
    struct reading in = {0};
    in.timeRead = timeCounter;
    in.temperaturOut = getOutsideTemp();
    in.light = getLight();
    in.humiditySoil = getRH();
    if(id > 0) //BME installed
    {
        in.temperaturIn = getBmeTemp();
        in.pressure = getBmePress();
    }
    if(id >=0x60)
        in.humidityAir = getBmeHumidity();
    return in;
}

void printReading(struct reading in)    //prints in to UART
{
    char str[16];
    for (uint8_t i = 0; i < 8; i++)
    {
        ultoa(readingIt(&in,i), str, 10);
        uartWriteString(str);
        if(i == 7)
            uartWriteString("\r\n");
        else
            uartWriteString(",");
    }
}

long selfDiagnosis(void)     //returns self diagnosis error code
{
    long errCode = 0;

    ADCA.CTRLA = ADC_ENABLE_bm;                     //Read Voltage at V_ref Pin. Should always be 4095 counts
    ADCA.CH1.CTRL |= ADC_CH_START_bm;
    while(!(ADCA.CH1.INTFLAGS & ADC_CH_CHIF_bm));
    ADCA.CH1.INTFLAGS = ADC_CH_CHIF_bm;
    if(ADCA.CH1.RES < 4090)
        errCode |= (1 << 1);
    ADCA.CTRLA &= ~ADC_ENABLE_bm;

    time_t timeTmp = timeCounter;                   //is the RTC running?
    _delay_ms(1010);
    if(timeCounter - timeTmp < 1)
        errCode |= (1 << 2);

    if(timeCounter < 1577833200)                    //is the RTC initialized (year after 2020)?
        errCode |= (1 << 3);

    if(flashID() != 0xBF258D)                      //Flash Signature as expected?
        errCode |= (1 << 4);

    if(flashAdr < (ADR_MAX - SECTOR_SIZE))                    //Flash read/write check only if last sector isn't already used
    {
        flashErase4kB(ADR_MAX - SECTOR_SIZE);
        uint8_t test = 0xAA;
        flashWrite(&test, 1, ADR_MAX - SECTOR_SIZE);
        flashRead(&test, 1, ADR_MAX - SECTOR_SIZE);
        if(test == 0xFF)                            //Not Erased properly
            errCode |= (1 << 5);
        if(test != 0xAA)                            //Read or Write went wrong
            errCode |= (1 << 6);
        flashErase4kB(ADR_MAX - SECTOR_SIZE);
    }

    if(!(PORTC.IN & (1 << 2)))                      //UART Rx and Tx should be high when nothing is transmitted
        errCode |= (1 << 7);
    if(!(PORTC.IN & (1 << 3)))
        errCode |= (1 << 8);

    unsigned char tTmp = getOutsideTemp();          //Temperatur Sensor Plausibility
    if(tTmp == 0 || tTmp > 250)
        errCode |= (1 << 9);

    if(lightFNom < 1100 || lightFNom > 10000)       //Light Frequency Range
        errCode |= (1 << 10);

    if(getIntervall == UINT16_MAX)                  //Sampling Intervall not set
        errCode |= (1 << 11);

    if(getTOutOff == UINT8_MAX)                     //Temperatur Offset not set
        errCode |= (1 << 12);

    if(id == 0)                                     //BME Connected?
        errCode |= (1 << 13);
    else                                            //BME Readings in range?
    {
        if(getBmeTemp() == 0 || getBmeTemp()  > 850 || getBmePress() < 3000 || getBmePress()  > 11000)
            errCode |= (1 << 14);
    }
    return errCode;
}

void quicksort(uint16_t *data, uint16_t n)       //sorts data till n ascending
{
  if (n < 2) return;

  uint16_t pivot = data[n / 2];

  int16_t i, j;
  for (i = 0, j = (int16_t) n - 1; ; i++, j--)
  {
    while (data[i] < pivot) i++;
    while (data[j] > pivot) j--;

    if (i >= j) break;

    uint16_t temp = data[i];
    data[i]     = data[j];
    data[j]     = temp;
  }

  quicksort(data, i);
  quicksort(data + i, n - i);
}

uint16_t getMedian(uint16_t *rd, const uint16_t n)   //returns Median of the Values in rd to rd[n]
{
    quicksort(rd, n);
	return n % 2 ? rd[n / 2] : (rd[n / 2 - 1] + rd[n / 2]) / 2;
}

uint8_t getOutsideTemp(void)  //returns Outside Temperatur in °C*5
{
    ADCA.CTRLA = ADC_ENABLE_bm;
    uint16_t tempArr[ADC_N];
    for(uint16_t i = 0; i < ADC_N; i++)
	{
		ADCA.CH0.CTRL |= ADC_CH_START_bm;
        while(!(ADCA.CH0.INTFLAGS & ADC_CH_CHIF_bm));
        ADCA.CH0.INTFLAGS = ADC_CH_CHIF_bm;
        tempArr[i] = ADCA.CH0.RES;
	}
    ADCA.CTRLA &= ~ADC_ENABLE_bm;
    return (getMedian(tempArr, ADC_N) / 8) + getTOutOff - 128;
}

uint16_t getLight(void)                         //returns illuminate in lux
{
    if(lightFNom < 1200)                        //Sensor cutoff Frequency
        return 0;
    return (unsigned int) ((66.46 * pow(1.732, (lightFNom / 1000.0)) ) - 133.5);
}

uint8_t getRH(void)                             //returns relativ humidity from external moisture sensor
{
    int16_t rh = 208 - (int16_t) (getExtCount()/7);       //f(x) = -1/7x+208
    return (rh > 100 || rh < 0) ? 0 : rh;
}

uint16_t getExtCount(void)  //returns ADC count from external Sensor
{
    ADCA.CTRLA = ADC_ENABLE_bm;
    uint16_t tempArr[ADC_N];
    for(uint16_t i = 0; i < ADC_N; i++)
	{
		ADCA.CH2.CTRL |= ADC_CH_START_bm;
        while(!(ADCA.CH2.INTFLAGS & ADC_CH_CHIF_bm));
        ADCA.CH2.INTFLAGS = ADC_CH_CHIF_bm;
        tempArr[i] = ADCA.CH2.RES;
	}
    ADCA.CTRLA &= ~ADC_ENABLE_bm;
    return getMedian(tempArr, ADC_N);
}

uint32_t readingIt(struct reading *v, uint8_t i) //makes it possible to iterate through a reading
{
    switch(i)
    {
        case 0: return v->timeRead;
        case 1: return v->light;
        case 2: return v->temperaturOut;
        case 3: return v->temperaturIn;
        case 4: return v->pressure;
        case 5: return v->humidityAir;
        case 6: return v->humiditySoil;
        default: return 0;
    }
}

ISR(USARTC0_RXC_vect)       //UART ISR
{
    char Data = USARTC0.DATA;
    USARTC0.DATA = Data;
    if(Data == '\r')
        USARTC0.DATA = '\n';
    static unsigned char i = 0;
    uartBuf[i] = Data;
    if(i == 15 || Data == '\r')
    {
        uartBuf[i] = '\0';
        instruct = true;
        i = 0;
    }
    else
        i++;
}

ISR(RTC_OVF_vect)          //RTC ISR
{
    timeCounter++;
    lightFNom = lightF;
    lightF = 0;
    if((timeCounter % getIntervall) == 0)
        takeMeasurement = true;
}

//ISR for Frequency Counter
ISR(PORTD_INT0_vect)
{
    lightF++;
}