//Writen by TjarkG and published under the MIT License

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __unix

#include <fcntl.h> 
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
        exit(2);
    }
    //baudrate 115200, 8 bits, no parity, 1 stop bit
    set_interface_attribs(fd, B115200);
}

void stopUART(void){}                 //not needed for unix

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

    rdlen = read(fd, buf, 64);
    if (rdlen > 0)
        buf[rdlen] = 0;
    else if (rdlen < 0)
        fprintf(stderr,"Error from read: %d: %s\n", rdlen, strerror(errno));
    else  // rdlen == 0
        fprintf(stderr,"Nothing read. EOF?\n");

    if(buf[0] == '\n')  //repeat read when only newline is found
        getUartLine(buf);
}

#endif //unix

#ifdef _WIN32

#include<windows.h>

HANDLE com;

void startUART(char *portname)      //opens UART portname
{
    char nameTmp[16];
    sprintf(nameTmp,"\\\\.\\%s", portname);
    com = CreateFileA(nameTmp,                //port name
                      GENERIC_READ | GENERIC_WRITE, //Read/Write
                      0,                            // No Sharing
                      0,                            // No Security
                      OPEN_EXISTING,                // Open existing port only
                      0,                            // Non Overlapped I/O
                      0);                           // Null for Comm Devices

    if (com == INVALID_HANDLE_VALUE)
    {
        fprintf(stderr,"Error opening %s: %s\n", portname, strerror(errno));
        exit(2);
    }

    DCB dcbConfig;

    if(GetCommState(com, &dcbConfig))
    {
        dcbConfig.BaudRate = CBR_115200;
        dcbConfig.ByteSize = 8;
        dcbConfig.Parity = NOPARITY;
        dcbConfig.StopBits = ONESTOPBIT;
        dcbConfig.fBinary = TRUE;
        dcbConfig.fParity = TRUE;
        dcbConfig.fInX = TRUE;
    }
    else
        fprintf(stderr,"Error opening %s: %s\n", portname, strerror(errno));

    if(!SetCommState(com, &dcbConfig))
        fprintf(stderr,"Error opening %s: %s\n", portname, strerror(errno));

    COMMTIMEOUTS commTimeout;

    if(GetCommTimeouts(com, &commTimeout))
    {
        commTimeout.ReadIntervalTimeout     = 1000 * 2;
        commTimeout.ReadTotalTimeoutConstant     = 1000 * 2;
        commTimeout.ReadTotalTimeoutMultiplier     = 1000 * 10;
        commTimeout.WriteTotalTimeoutConstant     = 1000 * 2;
        commTimeout.WriteTotalTimeoutMultiplier = 1000 * 10;
    }
    else
        fprintf(stderr,"Error opening %s: %s\n", portname, strerror(errno));

    if(!SetCommTimeouts(com, &commTimeout))
        fprintf(stderr,"Error opening %s: %s\n", portname, strerror(errno));
}

void stopUART(void)
{
    if(com != INVALID_HANDLE_VALUE)
    {
        CloseHandle(com);
        com = INVALID_HANDLE_VALUE;
    }
}

void printUART(const char *in)        //prints in to UART
{
    unsigned char length = strlen(in);
    unsigned char bytesSend = 0;

    while(bytesSend < length)
    {
        unsigned long bytesWritten;

        if(WriteFile(com, &in[bytesSend], 1, &bytesWritten, NULL) != 0)
        {
            if(bytesWritten > 0)
                ++bytesSend;
            else
                fprintf(stderr,"Error from write: %d, %s\n", bytesWritten, strerror(errno));
        }
        else
            fprintf(stderr,"Error from write: %d, %s\n", bytesWritten, strerror(errno));
    }
}

void getUartLine(char *buf)     //puts on line of UART input in buf
{
    unsigned long eventMask;

    if(!SetCommMask(com, EV_RXCHAR))
        fprintf(stderr,"Error from read: %s\n", strerror(errno));
    if(WaitCommEvent(com, &eventMask, NULL))
    {
        char szBuf;
        unsigned long rdlen;
        unsigned long rges = 0;
        do
        {
            if(ReadFile(com, &szBuf, 1, &rdlen, NULL) != 0)
            {
                if(rdlen > 0)
                {
                    buf[rges++] = szBuf;
                    if(szBuf == '\n')
                    {
                        break;
                    }
                }
            }
            else
                fprintf(stderr,"Error from read: %s\n", strerror(errno));
        } while(rdlen > 0);
        buf[rges] = '\0';
    }
    else
        fprintf(stderr,"Error from read: %s\n", strerror(errno));
}

#endif // WIN32