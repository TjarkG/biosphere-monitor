//Writen by TjarkG and published under the MIT License
//communicate via UART on Linux and Windows (half-working)

#include <cerrno>
#include <cstdio>
#include <cstring>

#ifdef __unix

#include <fcntl.h> 
#include <termios.h>
#include <unistd.h>

int fd;

int setInterfaceAttributs(unsigned int speed)
{
    struct termios tty{};

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
    tty.c_cflag &= ~CRTSCTS;    // no hardware flow control

    tty.c_lflag |= ISIG;
    tty.c_lflag &= ~(ECHO | ECHOE | ECHONL | IEXTEN);

    tty.c_iflag &= ~IGNCR;  // preserve carriage return 
    tty.c_iflag &= ~INPCK;
    tty.c_iflag &= ~(INLCR | ICRNL | IUCLC | IMAXBEL);
    tty.c_iflag &= ~(IXON | IXOFF | IXANY);   // no SW flow control

    tty.c_oflag &= ~OPOST;

    tty.c_cc[VEOL] = 0;
    tty.c_cc[VEOL2] = 0;
    tty.c_cc[VEOF] = 0x04;

    tty.c_cc[VTIME] = 20;   /* Set timeout of 2 seconds */
    tty.c_cc[VMIN] = 0;

    if (tcsetattr(fd, TCSANOW, &tty) != 0)
    {
        fprintf(stderr,"Error from tcsetattr: %s\n", strerror(errno));
        return -1;
    }
    return 0;
}

char startUART(char *portName)      //opens UART port name
{
    fd = open(portName, O_RDWR | O_NOCTTY | O_SYNC);
    if (fd < 0)
        return 2;
    //baudrate 1000000, 8 bits, no parity, 1 stop bit
    setInterfaceAttributs(B1000000);
    return 0;
}

void stopUART(){}                 //not needed for unix

void printUART(const char *in)        //prints in to UART
{
    auto length = strlen(in);
    auto wLen = write(fd, in, length);
    if (wLen != length)
        fprintf(stderr, "Error from write: %ld %d\n", wLen, errno);
    tcdrain(fd);    // delay for output
}

char getUartLine(char *buf)     //puts one line of UART input in buf
{
    auto readLen = read(fd, buf, 64);
    if (readLen > 0)
        buf[readLen] = 0;
    else if (readLen < 0)
        fprintf(stderr, "Error from read: %ld: %s\n", readLen, strerror(errno));
    else  //timeout
    {
        buf[0] = '\0';
        return -1;
    }

    if(buf[0] == '\n')  //repeat read when only newline is found
        getUartLine(buf);

    return 0;
}

#endif //__unix

#ifdef _WIN32

#include<windows.h>

HANDLE com;

void startUART(char *portName)      //opens UART port name
{
    char nameTmp[16];
    sprintf(nameTmp,"\\\\.\\%s", portName);
    com = CreateFileA(nameTmp,                //port name
                      GENERIC_READ | GENERIC_WRITE, //Read/Write
                      0,                            // No Sharing
                      0,                            // No Security
                      OPEN_EXISTING,                // Open existing port only
                      0,                            // Non Overlapped I/O
                      0);                           // Null for Comm Devices

    if (com == INVALID_HANDLE_VALUE)
    {
        fprintf(stderr,"Error opening %s: %s\n", portName, strerror(errno));
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
        fprintf(stderr,"Error opening %s: %s\n", portName, strerror(errno));

    if(!SetCommState(com, &dcbConfig))
        fprintf(stderr,"Error opening %s: %s\n", portName, strerror(errno));

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
        fprintf(stderr,"Error opening %s: %s\n", portName, strerror(errno));

    if(!SetCommTimeouts(com, &commTimeout))
        fprintf(stderr,"Error opening %s: %s\n", portName, strerror(errno));
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

#endif // _WIN32