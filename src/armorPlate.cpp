/*--------incldue-----------*/
#include "armorPlate.h"
/*--------------------------*/

/*------------变量区---------*/
double times;
const int Light_Area_min = 50;
const float Light_angle = 20;
const float Light_Aspect_ratio = 1.0;
const float Light_crown = 0.5;
/*--------------------------------*/

/********************************************************************************
版权：2023-2024，Climber 
函数名：lightBar
介绍: 灯条构造函数
参数：Rec(灯条的旋转矩阵)、j（第几个）
作者：
版本：2023.1.15.01
完成时间：2023.1.27
* ********************************************************************************/
lightBar::lightBar(RotatedRect Rec,int j){
    Rect = Rec;
	i = j;
}

/********************************************************************************
版权：2023-2024，Climber 
函数名：Solution
介绍: 算法构造函数
参数：
作者：
版本：2023.1.15.01
完成时间：2023.1.27
* ********************************************************************************/
Solution::Solution(bool COLOR){
	color = COLOR;
}

/********************************************************************************
版权：2023-2024，Climber 
函数名：Search
介绍: 装甲板识别
参数：src(原图像)
作者：
版本：2023.1.15.01
完成时间：2023.1.27
* ********************************************************************************/
void Solution::armorPlateSearch(cv::Mat src){
	std::vector<lightBar> Light_Bar;
	cv::Mat srcHsv,srcThread;
	cv::Mat dst;
	static int threadvlaue = 75;

	//cv::cvtColor(src, src, cv::COLOR_BayerRG2RGB);
	//cv::cvtColor(src, srcHsv, cv::COLOR_RGB2HSV);
	
	std::vector<cv::Mat> channels;
	cv::split(src, channels);
	if (color==RED){
		dst = channels[2] - channels[0];
	}
	else if(color==BLUE){
		dst = channels[0] - channels[2];
	}
	
	
	//cv::blur(dst, dst, cv::Size(1, 3));
	cv::threshold(dst, srcThread, threadvlaue, 255, cv::THRESH_BINARY);
	//cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(3, 3), cv::Point(-1, -1));
	//dilate(srcThread, srcThread, kernel);
	
	std::vector<std::vector<cv::Point>> Light_Contour; // 发现的轮廓
	findContours(srcThread, Light_Contour, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE); // 寻找轮廓
	if (Light_Contour.size() != 0){
		for (int i = 0; i < Light_Contour.size(); i++) {
			// 求轮廓面积
			float Light_Contour_Area = contourArea(Light_Contour[i]);
			// 去除较小轮廓&fitEllipse的限制条件
			if (Light_Contour_Area < Light_Area_min || Light_Contour[i].size() <= 3)
				continue;
			// 用椭圆拟合区域得到外接矩形
			cv::RotatedRect Light_Rec = fitEllipse(Light_Contour[i]);
			Light_Rec = adjustRec(Light_Rec);
			if (Light_Rec.angle > Light_angle)
				continue;
			// 长宽比和轮廓面积比限制
			if (Light_Rec.size.width / Light_Rec.size.height > Light_Aspect_ratio
				|| Light_Contour_Area / Light_Rec.size.area() < Light_crown)
				continue;
			//// 扩大灯柱的面积
			Light_Rec.size.height *= 1.1;
			Light_Rec.size.width *= 1.1;
			//保存灯条
			Light_Bar.push_back(lightBar(Light_Rec, i));

			//cv::drawContours(src, Light_Contour, i, cv::Scalar(0, 0, 255), 2);
		}
		//匹配灯条
		matchLight(Light_Bar, src, Light_Contour);
	}
	//imshow("src", src);
}

/********************************************************************************
版权：2023-2024，Climber 
函数名：matchLight
介绍: 灯条匹配
参数：light(所有合适的灯条)、src(原图像)、Light_Contour(灯条轮廓)
作者：
版本：2023.1.15.01
完成时间：2023.1.27
* ********************************************************************************/
void Solution::matchLight(std::vector<lightBar> light, cv::Mat src, std::vector<std::vector<cv::Point>> Light_Contour){
	static double times = 0;
	static double lasttime = 0;
	Point currentCenter;
	Point forcastPositon;
	std::vector<int*> reliability;
	double dist[3] = { 0 };
	double speed[3]={ 0 };
	static double lastSpeed[3] = { 0 };
	double t=0;
	double acc[3] = { 0 };
	if (light.size() < 2){
		printf("没有找到装甲板\n");
	}
	else{
		for (int i = 0; i < light.size() - 1; i++){
			for (int j = i + 1; j < light.size(); j++){
				int level = 0;
				double area[2];
				int temp[3];
				lightBar leftRect = light[i];
				lightBar rightRect = light[j];

				//角度判断
				if (leftRect.Rect.angle == rightRect.Rect.angle) {
					level += 10;
				}
				else if (abs(leftRect.Rect.angle - rightRect.Rect.angle) < 5) {
					level += 8;
				}
				else if (abs(leftRect.Rect.angle - rightRect.Rect.angle) < 10) {
					level += 6;
				}
				else if (abs(leftRect.Rect.angle - rightRect.Rect.angle) < 30) {
					level += 4;
				}
				else if (abs(leftRect.Rect.angle - rightRect.Rect.angle) < 90) {
					level += 1;
				}
				else {
					break;
				}

				//面积筛选
				area[0] = leftRect.Rect.size.width * leftRect.Rect.size.height;
				area[1] = rightRect.Rect.size.width * rightRect.Rect.size.height;
				if (area[0] == area[1]) {
					level += 10;
				}
				else if (min(area[0], area[1]) * 1.5 > max(area[0], area[1])) {
					level += 8;
				}
				else if (min(area[0], area[1]) * 2 > max(area[0], area[1])) {
					level += 6;
				}
				else if (min(area[0], area[1]) * 3 > max(area[0], area[1])) {
					level += 4;
				}
				else if (min(area[0], area[1]) * 4 > max(area[0], area[1])) {
					level += 1;
				}
				else {
					break;
				}
				//长度筛选
				double half_height = (leftRect.Rect.size.height + rightRect.Rect.size.height) / 4;
				if (leftRect.Rect.center.y == rightRect.Rect.center.y) {
					level += 10;
				}
				else if (abs(leftRect.Rect.center.y - rightRect.Rect.center.y) < 0.2 * half_height) {
					level += 8;
				}
				else if (abs(leftRect.Rect.center.y - rightRect.Rect.center.y) < 0.4 * half_height) {
					level += 6;
				}
				else if (abs(leftRect.Rect.center.y - rightRect.Rect.center.y) < 0.8 * half_height) {
					level += 4;
				}
				else if (abs(leftRect.Rect.center.y - rightRect.Rect.center.y) < half_height) {
					level += 1;
				}
				else {
					break;
				}
				temp[0] = i;
				temp[1] = j;
				temp[2] = level;
				reliability.push_back(temp);
			}
		}
	}
	if (reliability.empty()){
		printf("没有找到装甲板\n");
	}
	else{
		int maxLevel = 0, index = 0;
		for (int k = 0; k < reliability.size(); ++k){
			if (reliability[k][2] > maxLevel){
				maxLevel = reliability[k][2];
				index = k;
			}
		}
		Mat mask = Mat::zeros(src.size(), src.type());
		currentCenter.x = (light[reliability[index][0]].Rect.center.x + 
			light[reliability[index][1]].Rect.center.x) / 2;
		currentCenter.y = (light[reliability[index][0]].Rect.center.y + 
			light[reliability[index][1]].Rect.center.y) / 2;

		// circle(mask, currentCenter, 7.5, Scalar(0, 0, 255), 5);
		// //cv::drawContours(mask, Light_Contour, light[reliability[index][0]].i, cv::Scalar(0, 0, 255), 2);
		// //cv::drawContours(mask, Light_Contour, light[reliability[index][1]].i, cv::Scalar(0, 0, 255), 2);
		circle(src, Light_Contour[light[reliability[index][0]].i][0], 2, Scalar(255, 0, 0), 5);
		circle(src, Light_Contour[light[reliability[index][1]].i][0], 2, Scalar(0, 0, 255), 5);
		circle(src, light[reliability[index][0]].Rect.center, 2, Scalar(0, 255, 0), 5);
		circle(src, light[reliability[index][1]].Rect.center, 2, Scalar(255, 0, 255), 5);
		//cv::imshow("mask", mask);

		//已经匹配完成，开始计算

		std::vector<Point2f> P;//图像上的点
		P.clear();
		P.push_back((Point2f)Light_Contour[light[reliability[index][0]].i][0]);
		P.push_back((Point2f)Light_Contour[light[reliability[index][1]].i][0]);
		P.push_back((Point2f)light[reliability[index][0]].Rect.center);
		P.push_back((Point2f)light[reliability[index][1]].Rect.center);

		//将控制点在世界坐标系的坐标压入容器
		std::vector<cv::Point3f> objP;
		objP.clear();
		objP.push_back(cv::Point3f(-7.5, 3.0, 0));
		objP.push_back(cv::Point3f(7.5, 3.0, 0));
		objP.push_back(cv::Point3f(-7.5, 0, 0));
		objP.push_back(cv::Point3f(7.5, 0, 0));

		//创建旋转矩阵和平移矩阵
		Mat rvecs ;
		Mat tvecs ;

		//相机内参矩阵与外参矩阵(需要标定)
		Mat cameraMatrix = Mat::eye(3, 3, CV_64F);//单位矩阵函数
		cameraMatrix.at<double>(0, 0) = 3636.201;
		cameraMatrix.at<double>(0, 1) = 0;
		cameraMatrix.at<double>(0, 2) = 744.9806;
		cameraMatrix.at<double>(1, 0) = 0;
		cameraMatrix.at<double>(1, 1) =3642.7002;
		cameraMatrix.at<double>(1, 2) = 574.7465;
		cameraMatrix.at<double>(2, 0) = 0;
		cameraMatrix.at<double>(2, 1) = 0;
		cameraMatrix.at<double>(2, 2) = 1;

		Mat distCoeffs = Mat::zeros(5, 1, CV_64F);
		distCoeffs.at<double>(0, 0) = -0.0681;
		distCoeffs.at<double>(1, 0) = 0.4122;
		distCoeffs.at<double>(2, 0) = 0;
		distCoeffs.at<double>(3, 0) = 0;
		distCoeffs.at<double>(4, 0) = 0;
		//求解pnp
		static Mat lasttvecs = Mat::zeros(Size(1, 3), CV_64F);
		solvePnP(objP, P, cameraMatrix, distCoeffs, rvecs, tvecs);
		Mat rotM = Mat::eye(3, 3, CV_64F);
		Mat rotT = Mat::eye(3, 3, CV_64F);
		Rodrigues(rvecs, rotM);  //将旋转向量变换成旋转矩阵
		Rodrigues(tvecs, rotT);

		// //计算相机旋转角
		// double theta_x, theta_y, theta_z;
		// double PI = 3.14159;
		// theta_x = atan2(rotM.at<double>(2, 1), rotM.at<double>(2, 2));
		// theta_y = atan2(-rotM.at<double>(2, 0),
		// 	sqrt(rotM.at<double>(2, 1) * rotM.at<double>(2, 1) + rotM.at<double>(2, 2) * rotM.at<double>(2, 2)));
		// theta_z = atan2(rotM.at<double>(1, 0), rotM.at<double>(0, 0));
		// theta_x = theta_x * (180 / PI);
		// theta_y = theta_y * (180 / PI);
		// theta_z = theta_z * (180 / PI);

		//计算角度
		Mat adjustTvecs;
		Mat rot_camera2ptz = cv::Mat::eye(3, 3, CV_64FC1);
		Mat trans_camera2ptz = cv::Mat::zeros(3, 1, CV_64FC1);
		adjustTvecs = rot_camera2ptz * tvecs - trans_camera2ptz;
		adjustAngle(adjustTvecs,angle_x,angle_y,1,0);

		//计算深度
		Mat distance=Mat::zeros(Size(3,3), CV_64F);
		distance = rotM.t() * tvecs;

		//输出
		printf("距离： %.2lf\n", abs(distance.at<double>(2)));
		//printf("角度： theta_x:%.4lf ,theta_y:%.4lf ,theta_z:%.4lf\n", theta_x, theta_y, theta_z);
		//printf("angle_x:%.4lf ,angle_y:%.4lf\n", angle_x,angle_y);

		//计算速度和加速度
		
		t = ((times - lasttime)*1000) / CLOCKS_PER_SEC;
		
		dist[0] = (tvecs.at<double>(0) - lasttvecs.at<double>(0))*10; //单位mm
		speed[0] = dist[0] / t;

		dist[1] = (tvecs.at<double>(1) - lasttvecs.at<double>(1))*10;
		speed[1] = dist[1] / t;

		dist[2] = (tvecs.at<double>(2) - lasttvecs.at<double>(2))*10;
		speed[2] = dist[2] / t;

		acc[0] = (speed[0] - lastSpeed[0]) / t;
		acc[1] = (speed[1] - lastSpeed[1]) / t;
		acc[2] = (speed[2] - lastSpeed[2]) / t;
		//printf("速度：  speedx:%.4lf m/s,speedy:%.4lf m/s,speedz:%.4lf m/s\n", speed[0], speed[1], speed[2]);
		//printf("加速度：accx:%.4lf m/s^2,accy:%.4lf m/s^2,accz:%.4lf m/s^2\n", acc[0], acc[1], acc[2]);

		//计算预测点位置
		forcastPositon.x = currentCenter.x + speed[0] * t + 1 / 2 * acc[0] * t * t;
		forcastPositon.y = currentCenter.y + speed[1] * t + 1 / 2 * acc[1] * t * t;

		lasttvecs = tvecs;
		lastSpeed[0] = speed[0];
		lastSpeed[1] = speed[1];
		lastSpeed[2] = speed[2];
	}
	lasttime = times;
	times = clock();
}

/********************************************************************************
版权：2023-2024，Climber 
函数名：adjustRec
介绍: 调整灯条
参数：Light_Rec(灯条)
作者：
版本：2023.1.15.01
完成时间：2023.1.27
*********************************************************************************/
cv::RotatedRect Solution::adjustRec(cv::RotatedRect Light_Rec){
	if (Light_Rec.angle > 90)
		Light_Rec.angle = 180 - Light_Rec.angle;
	return Light_Rec;
}

/**
解算目标偏转角度.
*@copyright 2023-2024，Climber
*@name adjustAngle
*@param pos_in_ptz 输入位姿
*@param angle_x 输出-目标x偏角
*@param angle_y 输出-目标y偏角
*@param bullet_speed 输入-子弹速度
*@param current_ptz_angle 输入-当前偏角
*@author 郑龙
*@version 2023.2.15.01
*@date 2023.2.27
*/
void Solution::adjustAngle(const cv::Mat & pos_in_ptz, double & angle_x, double & angle_y, double bullet_speed, double current_ptz_angle)
{
	const double *_xyz = (const double *)pos_in_ptz.data;
	double down_t = 0.0;
	//cout << "pos_in_ptz " << pos_in_ptz << endl;
	if (bullet_speed > 10e-3)
		down_t = _xyz[2] / 100.0 / bullet_speed;
	//cout << "**********************************  "<< endl;
	//cout << "bullet_speed  " << bullet_speed << endl;
	//cout << "down_t  " << down_t;
	double offset_gravity = 0.5 * 9.8 * down_t * down_t * 100;
	offset_gravity=0;
	//cout << "offset_gravity  " << offset_gravity << endl;
	double xyz[3] = { _xyz[0], _xyz[1] - offset_gravity, _xyz[2] };
	//cout << "xyz[3]  " << xyz[0] << xyz[1] << xyz[2] << endl;
	double alpha = 0.0, theta = 0.0;
	//cout << "offset_y_barrel_ptz  " << offset_y_barrel_ptz << endl;
	//cout << "sqrt(xyz[1] * xyz[1] + xyz[2] * xyz[2])  " << sqrt(xyz[1] * xyz[1] + xyz[2] * xyz[2]) << endl;
    double offset_y_barrel_ptz=0;
	alpha = asin(offset_y_barrel_ptz / sqrt(xyz[1] * xyz[1] + xyz[2] * xyz[2]));
	//cout << "alpha  " << alpha << endl;
	if (xyz[1] < 0){
		//cout << "1" << endl;
		theta = atan(-xyz[1] / xyz[2]);
		angle_y = -(alpha + theta);  // camera coordinate
	}
	else if (xyz[1] < offset_y_barrel_ptz){
		//cout << "2" << endl;
		theta = atan(xyz[1] / xyz[2]);
		angle_y = -(alpha - theta);  // camera coordinate
	}
	else{
		//cout << "3  "<<endl;
		theta = atan(xyz[1] / xyz[2]);
		angle_y = (theta - alpha);   // camera coordinate
	}
	//cout << "theta  " << theta << endl;
	angle_x = atan2(xyz[0], xyz[2]);
	//cout << "angle_x: " << angle_x << "\tangle_y: " << angle_y <<  "\talpha: " << alpha << "\ttheta: " << theta << endl;
	angle_x = -angle_x * (180 / PI);
	angle_y = angle_y * (180 / PI);
	angle_x=90-angle_x;
	angle_y=90-angle_y;
	//cout << "angle_x=  " << 90-angle_x<< "angle_y=  " << 90-angle_y<< endl;
}

/**
PNP解算.
*@copyright 2023-2024，Climber
*@name pnpProcess
*@param p 图像上的点
*@param objP 实际物体上的点
*@param cameraMatrix 相机内参
*@param distCoeffs 相机外参
*@param current_ptz_angle 输入-当前偏角
*@author 郑龙
*@version 2023.2.15.01
*@date 2023.2.27
*/
void Solution::pnpProcess(vector<Point2f> P,vector<Point3f> objP,Mat cameraMatrix,Mat distCoeffs)
{

		//创建旋转矩阵和平移矩阵
		Mat rvecs ;
		Mat tvecs ;

		//求解pnp
		static Mat lasttvecs = Mat::zeros(Size(1, 3), CV_64F);
		solvePnP(objP, P, cameraMatrix, distCoeffs, rvecs, tvecs);
		Mat rotM = Mat::eye(3, 3, CV_64F);
		Mat rotT = Mat::eye(3, 3, CV_64F);
		Rodrigues(rvecs, rotM);  //将旋转向量变换成旋转矩阵
		Rodrigues(tvecs, rotT);

		// //计算相机旋转角
		// double theta_x, theta_y, theta_z;
		// double PI = 3.14159;
		// theta_x = atan2(rotM.at<double>(2, 1), rotM.at<double>(2, 2));
		// theta_y = atan2(-rotM.at<double>(2, 0),
		// 	sqrt(rotM.at<double>(2, 1) * rotM.at<double>(2, 1) + rotM.at<double>(2, 2) * rotM.at<double>(2, 2)));
		// theta_z = atan2(rotM.at<double>(1, 0), rotM.at<double>(0, 0));
		// theta_x = theta_x * (180 / PI);
		// theta_y = theta_y * (180 / PI);
		// theta_z = theta_z * (180 / PI);

		//计算角度
		Mat adjustTvecs;
		Mat rot_camera2ptz = cv::Mat::eye(3, 3, CV_64FC1);
		Mat trans_camera2ptz = cv::Mat::zeros(3, 1, CV_64FC1);
		adjustTvecs = rot_camera2ptz * tvecs - trans_camera2ptz;
		adjustAngle(adjustTvecs,angle_x,angle_y,1,0);

		//计算深度
		Mat distance=Mat::zeros(Size(3,3), CV_64F);
		distance = rotM.t() * tvecs;

		//输出
		printf("距离： %.2lf\n", abs(distance.at<double>(2)));
		//printf("角度： theta_x:%.4lf ,theta_y:%.4lf ,theta_z:%.4lf\n", theta_x, theta_y, theta_z);
		//printf("angle_x:%.4lf ,angle_y:%.4lf\n", angle_x,angle_y);
}