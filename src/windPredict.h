/********************************************************************************
版权：2023-2024，Climber 
文件名：windPredict.h
介绍:打风车时找到需要打击的位置,并预测其运动轨迹,给出判断
作者：袁骁
版本：2023.1.15.01
完成时间：2023.1.15
* ********************************************************************************/

#ifndef _WINDPREDICT_H_
#define _WINDPREDICT_H_ 
/*---------------include--------------*/
#include <iostream>
#include <opencv.hpp>
#include <math.h>
#include <ctime> 
/*------------------------------------*/

/*------------namespace---------------*/
using namespace std;
using namespace cv;
/*-----------------------------------*/

/*--------------define----------------*/
#define pointSetnumeber 5/*点集大小*/
#define speedSetnumber 2/*速度集合大小*/
#define blue 0/*蓝色*/
#define red 1/*红色*/
/*-----------------------------------*/

/*-----------------变量声明区----------------------*/
extern Point2f pointSet[pointSetnumeber];/*定义点集,存放目标点*/
extern int pointNumber;/*配合pointSet[pointSetnumeber]，存放点*/
extern int runNumber;/*存放目标点的次数，次数达标开始预测*/
extern float speed;
extern float acceleratedSpeed;/*速度，加速度*/
extern float speedSet[speedSetnumber];/*速度集合*/
extern int speedNumber;/*配合speedSet[speedSetnumber]，存放速度*/
extern float predictdistance;
extern Point2f predictPoint;/*定义预测距离和预测点*/
extern float lastDistance;/*初始化距离*/
extern int frame;/*帧数*/
extern int color;/*控制识别颜色，0代表蓝色，1代表红色*/
/*-------------------------------------------------------------------------*/

/*------------------------函数声明------------------------------------*/
float distance(Point2f lastPoint, Point2f presentPoint);/*计算两点间的距离*/
Point2f predict(Mat image,int runNumber,int color);/*预测目标点*/
/*---------------------------------------------------------------------*/

#endif