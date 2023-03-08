
#ifndef _ARMORPLATE_H
#define _ARMORPLATE_H
/*---------include-----------*/
#include <opencv.hpp>
#include <iostream>
/*---------------------------*/

/*---------define------------*/
#define RED true
#define BLUE false
#define PI 3.1415926
/*---------------------------*/

/*---------namespace---------*/
using namespace cv;
using namespace std;
/*---------------------------*/

/*--灯条类--*/
/*--用于存储识别到的灯条--*/
class lightBar{
public:
    lightBar(RotatedRect Rec,int j);
    RotatedRect Rect;
    int i;
};

/*--灯条匹配及预测计算--*/
class Solution{
private:
	bool color;
public:
	Solution(bool COLOR);
	void armorPlateSearch(cv::Mat src);
	void matchLight(std::vector<lightBar> Light_Bar, Mat src, std::vector<std::vector<cv::Point>> Light_Contour);
	void adjustAngle(const cv::Mat & pos_in_ptz, double & angle_x, double & angle_y, double bullet_speed, double current_ptz_angle);
	cv::RotatedRect adjustRec(cv::RotatedRect Light_Rec);
	void pnpProcess(vector<Point2f> P,vector<Point3f> objP,Mat cameraMatrix,Mat distCoeffs);
	double angle_x,angle_y;
};

/*--装甲板类--*/
class armorPlate{
private:
	bool color;
public:
    int Index=0;
	double distance;
	double speed;
	double acceleration;


};


#endif