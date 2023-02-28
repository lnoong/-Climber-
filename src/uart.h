#ifndef _UART_H
#define _UART_H

#pragma once
/*-------include--------*/ 
#include<iostream>
#include <unistd.h>
#include <termios.h>
#include<string.h>
# include "fcntl.h"
/*-----------------------*/

/*-------namespace-------*/
using namespace std;
/*-----------------------*/

/*--------------define----------------*/
#define sendNum 7
#define receiveNum 10
#define portName "/dev/ttyACM0"
/*-----------------------------------*/


/*-------变量区----------*/
/*--装甲版角度类--*/
/*--用于存储解算出装甲版的俯仰角及偏航角--*/
class angleUart{
public:
    int xH;
    int xL;
    int yH;
    int yL;
};
/*-----------------------*/

/*--------------函数声明-------------*/

int openPort() ;
int initPort(int fd,int nSpeed, int nBits, char nEvent, int nStop) ;
void uartSend(int fd,char sendBuf[], int length);
void uartReceive(int fd,char receiveBuf[], int length);
int charToHex(char c);
void angleDeal(double x,double y,angleUart &angle);

/*-------------------------------------*/

#endif