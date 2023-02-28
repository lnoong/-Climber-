/***************************************************************************************************
版权：2023-2024，Climber 
文件名：main.cpp
介绍:程序启动入口
作者：
版本：2023.1.20.01
完成时间：2023.1.20
***************************************************************************************************/
/*----------include------------*/
#include <opencv2/opencv.hpp>
#include "camera.h"
#include <thread>
#include "windPredict.h"
#include "armorPlate.h"
#include "uart.h"
/*-----------------------------*/

/*----------namespace----------*/
using namespace cv;
/*-----------------------------*/
int main(){
	/*------------变量区---------*/
	Mat src;			//源图像
	double start = 0;	//时间计算变量
	double finish = 0;
	double times = 0;
	char sendBuf[]={0xAA,0x00,0x00,0x00,0x00,0x00,0xFE};
	char receiveBuf[]={0xAA,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFE};
	/*---------------------------*/
	//打开相机
    OpenDevice();

    //开启采集图像线程
	thread thread0(GetImgThread);
	
	//打开串口通信; 
	int fd=openPort() ;
	waitKey(50);
	initPort(fd,9600, 8, 'N', 1) ;
	waitKey(50);

	// //通讯接收线程
	// thread thread1(uartReceive,fd,receiveBuf,sizeof(receiveBuf));
	waitKey(500);
	//主循环
	Mat img;
	while (1)
	{
		start=clock();
		ReadImgBuffer(src);
		//resize(src,img,Size(src.cols*0.5,src.rows*0.5));
		//pyrDown(src,img,Size(src.cols*0.5,src.rows*0.5));
		img=src;
		Solution cam(BLUE);
		cam.armorPlateSearch(img);
		angleUart angle;
		angleDeal(cam.angle_x,cam.angle_y, angle);
		sendBuf[1]=angle.xH,sendBuf[2]=angle.xL,sendBuf[3]=angle.yH,sendBuf[4]=angle.yL;
		uartSend(fd,sendBuf,sizeof(sendBuf));
		printf("angle_x%d.%d,angle_y%d.%d",angle.xH,angle.xL,angle.yH,angle.yL);

		imshow("img",img);
		waitKey(1);
		

		finish = clock();
		times = double(finish - start) / CLOCKS_PER_SEC*1000;
		printf("FPS:%.2lf\n", 1000 / times);
	}
    return 0;
}