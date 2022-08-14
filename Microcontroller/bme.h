//Writen by TjarkG and published under the MIT License

#ifndef bme_H_
#define bme_H_

extern enum Type {BMP280 = 0x58, BME280 = 0x60, BME680 = 0x61}  __attribute__ ((__packed__)) id;      //Sensor ID

void bmeInit(void);
unsigned int getBmeTemp(void);
unsigned int getBmePress(void);
unsigned char getBmeHumidity(void);

#endif //bme_H_