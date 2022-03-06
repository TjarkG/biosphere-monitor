//Writen by TjarkG and published under the MIT License
//communicate via UART on Linux and Windows (half-working)

#ifndef tty_H_
#define tty_H_

#ifdef _WIN32

#include <windows.h>

#endif // WIN32

#ifdef __unix

#include <fcntl.h> 
#include <termios.h>
#include <unistd.h>

#endif //unix

void startUART(char *portname);                 //opens UART portname
void stopUART(void);                            //not needed for unix
void printUART(const char *in);                 //prints in to UART
void getUartLine(char *buf);                    //puts on line of UART input in buf


#endif //tty_H_