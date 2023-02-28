/********************************************************************************
版权：2023-2024，Climber 
文件名：camera.h
介绍:打开设备以及相机初始化的相关操作
作者：
版本：2023.1.20.01
完成时间：2023.1.20
*********************************************************************************/
#ifndef _CAMERA_H
#define _CAMERA_H

#pragma once
/*-------include--------*/ 
#include "/opt/MVS/include/MvCameraControl.h"
#include <opencv.hpp>
#include <stdio.h>
#include "string.h"
/*-----------------------*/

/*-------namespace-------*/
using namespace std;
/*-----------------------*/

/*-------变量区----------*/
enum CONVERT_TYPE
{
    OpenCV_Mat = 0,    // Most of the time, we use 'Mat' format to store image data after OpenCV V2.1
    OpenCV_IplImage = 1,   //we may also use 'IplImage' format to store image data, usually before OpenCV V2.1
};
/*-----------------------*/

/*--------------函数声明-------------*/
bool PrintDeviceInfo(MV_CC_DEVICE_INFO* pstMVDevInfo);
int RGB2BGR(unsigned char* pRgbData, unsigned int nWidth, unsigned int nHeight);
cv::Mat Convert2Mat(MV_FRAME_OUT_INFO_EX* pstImageInfo, unsigned char* pData);
int OpenDevice();
bool GetImgThread();
bool ReadImgBuffer(cv::Mat& srcImage);
int CloseDevice();
bool InitCamera();
bool SetWidthtHeight(int width, int height);
bool SetExposureTime(float dfFloatValue);
/*-------------------------------------*/

#endif
