// test2.cpp : 定义控制台应用程序的入口点。
//
#include "stdafx.h"
#include <opencv2\opencv.hpp>
#include <iostream>
#include <string>
#include "AngleSolver.h"

using namespace cv;
using namespace std;

Mat frame,org;  //定义一个Mat变量，用于存储每一帧的图像  
double time0;
int exposure = -1;
double pitch_now = 0;
float camera_m[3][3] = { { 1028.4, 0, 643.9749 }, { 0, 1029.4, 660.8618 }, { 0, 0, 1 } };
float dist_c[1][5] = { -0.4214, 0.2319 ,  0, 0 ,0 };
Mat camera_matrix = Mat(3, 3, CV_32FC1, camera_m);
Mat dist_coeff = Mat(1, 5, CV_32FC1, dist_c);
AngleSolverFactory anglesolverfactory;
AngleSolver anglesolver = AngleSolver(camera_matrix, dist_coeff, 100, 100,1,0,4000);//传递摄像头内参畸变与目标大小确定距离

void on_mouse(int event, int x, int y, int flags, void *ustc)//event鼠标事件代号，x,y鼠标坐标，flags拖拽和键盘操作的代号  
{
	double angle_x = 0;
	double angle_y = 0;
	static Point pre_pt = (-1, -1);//初始坐标  
	if (event == CV_EVENT_LBUTTONDOWN)// 
	{
		pre_pt = Point(x, y);
		//cout << "x=  " << x << "y=  " << y << endl;
		RotatedRect rect(Point2f(x, y), Size2f(100, 100), 0);
		//Point2f target2d[4];
		anglesolverfactory = AngleSolverFactory(&anglesolver);
		anglesolverfactory.getAngle(rect, anglesolverfactory.TARGET_SAMLL_ATMOR, angle_x, angle_y, 14, pitch_now, Point2f(0, 0));
		pitch_now = angle_y;
	}
}

int main(int argc, _TCHAR* argv[]){
	//cout << camera_matrix << endl;
	//cout << dist_coeff << endl;
	//【1】从摄像头读入视频  
	VideoCapture capture(0);
	if (false == capture.isOpened())
	{
		return -1;
	}
	exposure = capture.get(CV_CAP_PROP_EXPOSURE);
	//显示曝光值  
	cout << ">设置前: 曝光值= " << exposure << endl;
	//设置曝光值  
	capture.set(CV_CAP_PROP_EXPOSURE, exposure);
	exposure = capture.get(CV_CAP_PROP_EXPOSURE);
	//显示曝光值  
	cout << ">设置后: 曝光值= " << exposure << endl;
	double width = capture.get(CV_CAP_PROP_FRAME_WIDTH);
	double height = capture.get(CV_CAP_PROP_FRAME_HEIGHT);
	//显示尺寸  
	cout << ">宽：= " << width << ";  高: =" << height << endl;
	//【2】循环显示每一帧
	cvNamedWindow("GGB", 1);
	while (1)
	{

		time0 = static_cast<double>(getTickCount());//记录起始时间  
		capture >> frame;  //读取当前帧  
		//若视频播放完成，退出循环  
		if (frame.empty())
		{
			break;
		}
		imshow("GGB", frame);  //显示当前帧
		setMouseCallback("GGB",on_mouse, 0);
		//cvShowImage("src", frame);
		//显示帧率  
		//cout << ">帧率= " << getTickFrequency() / (getTickCount() - time0) << endl;  reinterpret_cast<void*> (&frame)
		char c = (char)waitKey(10);
		if (c == 27)
			break;
	}
	return 0;
}

