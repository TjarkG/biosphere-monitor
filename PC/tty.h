/*
 * tty.h
 *
 * Header to communicate via UART on Linux
 *
 * Created: 30.08.2021 16:37:17
 *  Author: Tjark Gaudich
 */

#ifndef tty_H_
#define tty_H_

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __unix

#ifdef _WIN32

#include <windows.h>

#endif // WIN32

#include <fcntl.h> 
#include <termios.h>
#include <unistd.h>

#endif //unix

void startUART(char *portname);                 //opens UART portname
void stopUART(void);                            //not needed for unix
void printUART(const char *in);                 //prints in to UART
void getUartLine(char *buf);                    //puts on line of UART input in buf


#endif //tty_H_