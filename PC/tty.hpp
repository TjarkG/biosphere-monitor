//Writen by TjarkG and published under the MIT License
//communicate via UART on Linux and Windows (half-working)

#ifndef tty_HPP_
#define tty_HPP_

char startUART(char *portName);                 //opens UART port name
void stopUART();                            //not needed for unix
void printUART(const char *in);                 //prints in to UART
char getUartLine(char *buf);                    //puts one line of UART input in buf


#endif //tty_HPP_