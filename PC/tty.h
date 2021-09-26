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
#include <fcntl.h> 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

int fd;
int wlen;

int set_interface_attribs(int fd, int speed)
{
    struct termios tty;

    if (tcgetattr(fd, &tty) < 0)
    {
        fprintf(stderr,"Error from tcgetattr: %s\n", strerror(errno));
        return -1;
    }

    cfsetospeed(&tty, (speed_t)speed);
    cfsetispeed(&tty, (speed_t)speed);

    tty.c_cflag |= CLOCAL | CREAD;
    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= CS8;         // 8-bit characters
    tty.c_cflag &= ~PARENB;     // no parity bit
    tty.c_cflag &= ~CSTOPB;     // only need 1 stop bit
    tty.c_cflag &= ~CRTSCTS;    // no hardware flowcontrol

    tty.c_lflag |= ICANON | ISIG;  // canonical input
    tty.c_lflag &= ~(ECHO | ECHOE | ECHONL | IEXTEN);

    tty.c_iflag &= ~IGNCR;  // preserve carriage return 
    tty.c_iflag &= ~INPCK;
    tty.c_iflag &= ~(INLCR | ICRNL | IUCLC | IMAXBEL);
    tty.c_iflag &= ~(IXON | IXOFF | IXANY);   // no SW flowcontrol

    tty.c_oflag &= ~OPOST;

    tty.c_cc[VEOL] = 0;
    tty.c_cc[VEOL2] = 0;
    tty.c_cc[VEOF] = 0x04;

    if (tcsetattr(fd, TCSANOW, &tty) != 0)
    {
        fprintf(stderr,"Error from tcsetattr: %s\n", strerror(errno));
        return -1;
    }
    return 0;
}

void startUART(char *portname)      //opens UART portname
{
    fd = open(portname, O_RDWR | O_NOCTTY | O_SYNC);
    if (fd < 0)
    {
        fprintf(stderr,"Error opening %s: %s\n", portname, strerror(errno));
        return;
    }
    //baudrate 115200, 8 bits, no parity, 1 stop bit
    set_interface_attribs(fd, B115200);
}

void printUART(const char *in)        //prints in to UART
{
    char length = strlen(in);
    char wlen = write(fd, in, length);
    if (wlen != length)
        fprintf(stderr,"Error from write: %d, %d\n", wlen, errno);
    tcdrain(fd);    // delay for output
}

void getUartLine(char *buf)     //puts on line of UART input in buf
{
    unsigned char *p;
    int rdlen;

    rdlen = read(fd, buf, 128);
    if (rdlen > 0)
        buf[rdlen] = 0;
    else if (rdlen < 0)
        fprintf(stderr,"Error from read: %d: %s\n", rdlen, strerror(errno));
    else  // rdlen == 0
        fprintf(stderr,"Nothing read. EOF?\n");

    if(buf[0] == '\n')  //repeat read when only newline is found
        getUartLine(buf);
}

#endif //tty_H_