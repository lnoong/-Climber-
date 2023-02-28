#include "uart.h"
/*--------------------------------*/
/*------------变量区---------*/


/*--------------------------------*/

/**
打开串口.
*@copyright 2023-2024，Climber
*@name openPort
*@author 郑龙
*@version 2023.2.15.01
*@date 2023.2.27
*/
int openPort() 
{ 
    int fd;
	char *dev={};
    dev={portName};
	fd = open(dev, O_RDWR|O_NOCTTY|O_NDELAY); 
	if (-1 == fd)
		{ 
			perror("无法打开串口"); 
			return(-1); 
		} 
     /*恢复串口为阻塞状态*/ 
     if(fcntl(fd, F_SETFL, 0)<0) 
     		printf("fcntl failed!\n"); 
     else 
		printf("fcntl=%d\n",fcntl(fd, F_SETFL,0)); 
     return fd; 
}

/**
初始化串口.
*@copyright 2023-2024，Climber
*@name initPort
*@param fd openPort函数的返回值
*@param nSpeed 波特率
*@param nBits 数据位数7/8
*@param nEvent 奇偶校验 N/n无奇偶校验 O/o奇校验 E/e偶校验
*@param nStop 停止位
*@author 郑龙
*@version 2023.2.15.01
*@date 2023.2.27
*/
int initPort(int fd,int nSpeed, int nBits, char nEvent, int nStop) 
{ 
     struct termios newtio,oldtio; 
/*保存测试现有串口参数设置，在这里如果串口号等出错，会有相关的出错信息*/ 
     if  ( tcgetattr( fd,&oldtio)  !=  0) {  
      perror("SetupSerial 1");
	printf("tcgetattr( fd,&oldtio) -> %d\n",tcgetattr( fd,&oldtio)); 
      return -1; 
     } 
     bzero( &newtio, sizeof( newtio ) ); 
/*步骤一，设置字符大小*/ 
     newtio.c_cflag  |=  CLOCAL | CREAD;  
     newtio.c_cflag &= ~CSIZE;  
/*设置停止位*/ 
     switch( nBits ) 
     { 
     case 7: 
      newtio.c_cflag |= CS7; 
      break; 
     case 8: 
      newtio.c_cflag |= CS8; 
      break; 
     } 
/*设置奇偶校验位*/ 
     switch( nEvent ) 
     { 
     case 'o':
     case 'O': //奇数 
      newtio.c_cflag |= PARENB; 
      newtio.c_cflag |= PARODD; 
      newtio.c_iflag |= (INPCK | ISTRIP); 
      break; 
     case 'e':
     case 'E': //偶数 
      newtio.c_iflag |= (INPCK | ISTRIP); 
      newtio.c_cflag |= PARENB; 
      newtio.c_cflag &= ~PARODD; 
      break;
     case 'n':
     case 'N':  //无奇偶校验位 
      newtio.c_cflag &= ~PARENB; 
      break;
     default:
      break;
     } 
     /*设置波特率*/ 
switch( nSpeed ) 
     { 
     case 2400: 
      cfsetispeed(&newtio, B2400); 
      cfsetospeed(&newtio, B2400); 
      break; 
     case 4800: 
      cfsetispeed(&newtio, B4800); 
      cfsetospeed(&newtio, B4800); 
      break; 
     case 9600: 
      cfsetispeed(&newtio, B9600); 
      cfsetospeed(&newtio, B9600); 
      break; 
     case 115200: 
      cfsetispeed(&newtio, B115200); 
      cfsetospeed(&newtio, B115200); 
      break; 
     case 460800: 
      cfsetispeed(&newtio, B460800); 
      cfsetospeed(&newtio, B460800); 
      break; 
     default: 
      cfsetispeed(&newtio, B9600); 
      cfsetospeed(&newtio, B9600); 
     break; 
     } 
/*设置停止位*/ 
     if( nStop == 1 ) 
      newtio.c_cflag &=  ~CSTOPB; 
     else if ( nStop == 2 ) 
      newtio.c_cflag |=  CSTOPB; 
/*设置等待时间和最小接收字符*/ 
     newtio.c_cc[VTIME]  = 0; 
     newtio.c_cc[VMIN] = 0; 
/*处理未接收字符*/ 
     tcflush(fd,TCIFLUSH); 
/*激活新配置*/ 
if((tcsetattr(fd,TCSANOW,&newtio))!=0) 
     { 
      perror("com set error"); 
      return -1; 
     } 
     printf("set done!\n"); 
     return 0; 
} 

/**
向串口发送数据.
*@copyright 2023-2024，Climber
*@name uartSend
*@param fd openPort函数的返回值
*@param sendBuf 要发送的字符数组
*@param length 字符数组长度
*@author 郑龙
*@version 2023.2.15.01
*@date 2023.2.27
*/
void uartSend(int fd,char sendBuf[], int length)
{
    int w;
    w = write(fd, sendBuf, length);
    if(w == -1)
    {
        printf("Send failed!\n");
    }
    else
    {
        printf("Send success!\n");
    }
}

/**
向串口发送数据.
*@copyright 2023-2024，Climber
*@name uartReceive
*@param fd openPort函数的返回值
*@param sendBuf 要接收的字符数组
*@param length 字符数组长度
*@author 郑龙
*@version 2023.2.15.01
*@date 2023.2.27
*/
void uartReceive(int fd,char receiveBuf[], int length)
{
    tcflush(fd, TCIFLUSH);
    int r;
    char receiveBuf_inside[receiveNum*3-1];
    while(1)
    {
     r = read(fd, receiveBuf_inside, sizeof(receiveBuf_inside));
     for(int i=0;i<receiveNum;i++)
     {
          int head=charToHex(receiveBuf_inside[i*3])*16;
          int end=charToHex(receiveBuf_inside[i*3+1]);
          int data=head+end;
          receiveBuf[i]=data;
     }
    }
}

/**
HEX样式的char转int.
*@copyright 2023-2024，Climber
*@name charToHex
*@param c 要转化的字符
*@return char对应的int
*@author 郑龙
*@version 2023.2.15.01
*@date 2023.2.27
*/
int charToHex(char c)
{
    if((c>='0')&&(c<='9'))
        return c-'0';     //将0-9的数字字符转为十六进制格式
    else if((c>='A')&&(c<='F'))
        return c-'A'+10;  //将A-F的字符转为十六进制格式，例如字符'C'-'A'+10=12=0x0C
    else if((c>='a')&&(c<='f'))
        return c-'a'+10;   //将a-f的字符转为十六进制格式
    else
        return 0x00;
}

/**
小数角度转整形(去小数点).
*@copyright 2023-2024，Climber
*@name angleDeal
*@param x x方向角度
*@param y y方向角度
*@param angle 用于保存返回的角度
*@author 郑龙
*@version 2023.2.15.01
*@date 2023.2.27
*/
void angleDeal(double x,double y,angleUart &angle)
{
		angle.xH=(int)x;
		angle.xL=(int)((x-angle.xH)*100);
		angle.yH=(int)y;
		angle.yL=(int)((y-angle.yH)*100);
}

