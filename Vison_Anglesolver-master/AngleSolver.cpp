#include "stdafx.h"
#include "AngleSolver.h"
#include "opencv2/opencv.hpp"

using namespace cv;
using namespace std;
//获取图像坐标对应的世界坐标并计算PnP
void RectPnPSolver::solvePnP4Points(const std::vector<cv::Point2f> & points2d, cv::Mat & rot, cv::Mat & trans){
	if (width_target < 10e-5 || height_target < 10e-5){
		rot = cv::Mat::eye(3, 3, CV_64FC1);
		trans = cv::Mat::zeros(3, 1, CV_64FC1);
		cout << "GG" << endl;
		return;
	}
	std::vector<cv::Point3f> point3d;
	float half_x = width_target / 2.0;
	float half_y = height_target / 2.0;

	point3d.push_back(Point3f(-half_x, -half_y, 0));
	point3d.push_back(Point3f(half_x, -half_y, 0));
	point3d.push_back(Point3f(half_x, half_y, 0));
	point3d.push_back(Point3f(-half_x, half_y, 0));
	//cout << "half_x" << half_x;
	//cout << "half_y" << half_y;
	cv::Mat r;
	//cout << "cam_matrix  " << cam_matrix << endl;
    //cout << "distortion_coeff  " << distortion_coeff << endl;
	//cv::solvePnP() 
	cv::solvePnP(point3d, points2d, cam_matrix, distortion_coeff, r, trans);//transÆ½ÒÆ¾ØÕó
	Rodrigues(r, rot);//rot==Ðý×ª¾ØÕó
}



void AngleSolver::setRelationPoseCameraPTZ(const cv::Mat & rot_camera_ptz, const cv::Mat & trans_camera_ptz, double y_offset_barrel_ptz) {
	rot_camera_ptz.copyTo(rot_camera2ptz);
	trans_camera_ptz.copyTo(trans_camera2ptz);
	offset_y_barrel_ptz = y_offset_barrel_ptz;
}


bool AngleSolver::getAngle(const cv::RotatedRect & rect, double & angle_x, double & angle_y, double bullet_speed, double current_ptz_angle, const cv::Point2f & offset){
	if (rect.size.height < 1)
		return false;

	// get 3D positon in camera coordinate
	//    double wh_ratio = width_target/height_target;
	//    RotatedRect adj_rect(rect.center, Size2f(rect.size.width, rect.size.width/wh_ratio), rect.angle);
	//    vector<Point2f> target2d;
	//    getTarget2dPoinstion(adj_rect, target2d, offset);

	vector<Point2f> target2d;
	getTarget2dPoinstion(rect, target2d, offset);

	cv::Mat r;
	//cout << "position_in_camera  " << position_in_camera << endl;
	//width_target = 50;
	//height_target = 100;
	//setTargetSize(10, 10);// todo
	//cout << "width_target  " << width_target << endl;
	RectPnPSolver::solvePnP4Points(target2d, r, position_in_camera);//£¿£¿£¿£¿£¿£¿£¿£¿£¿£¿£¿£¿£¿£¿£¿£¿£¿£¿£¿£¿£¿£¿<
	//cout << "position_in_camera  " << position_in_camera << endl;
	//cout << "position_in_camera  " << position_in_camera << endl;
	//position_in_camera.at<double>(2, 0) = 1.4596 * position_in_camera.at<double>(2, 0);  // for camera-2 calibration (unfix center)
	//position_in_camera.at<double>(2, 0) = 1.5348 * position_in_camera.at<double>(2, 0);  // for camera-MH calibration (unfix center)
	position_in_camera.at<double>(2, 0) = scale_z * position_in_camera.at<double>(2, 0);
	if (position_in_camera.at<double>(2, 0) < min_distance || position_in_camera.at<double>(2, 0) > max_distance){
		cout << "out of range: [" << min_distance << ", " << max_distance << "]\n";
		//cout << "position_in_camera.at<double>(2, 0)  " << position_in_camera.at<double>(2, 0) << endl;
		return false;
	}

	// translate camera coordinate to PTZ coordinate
	//position_in_ptz = position_in_camera;
	tranformationCamera2PTZ(position_in_camera, position_in_ptz);//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	//cout << "********************************" << endl;
	//cout << "position_in_came  " << position_in_camera << endl;
	//cout << "position_in_ptz  " << position_in_ptz << endl;
	//    cout << "position_in_camera: " << position_in_camera.t() << endl;
	//    cout << "position_in_ptz: " << position_in_ptz.t() << endl;
	// calculte angles to turn, so that make barrel aim at target
	adjustPTZ2Barrel(position_in_ptz, angle_x, angle_y, bullet_speed, current_ptz_angle);

	return true;
}
//将相机坐标系转换为炮台坐标系
void AngleSolver::tranformationCamera2PTZ(const cv::Mat & pos, cv::Mat & transed_pos){
	transed_pos = rot_camera2ptz * pos - trans_camera2ptz;
	//cout << "********************************" << endl;
	//cout << rot_camera2ptz << endl;
	//cout << trans_camera2ptz << endl;
}
//获取到solvePnP的输出矩阵后根据公式计算出角度值
void AngleSolver::adjustPTZ2Barrel(const cv::Mat & pos_in_ptz, double & angle_x, double & angle_y, double bullet_speed, double current_ptz_angle){
	const double *_xyz = (const double *)pos_in_ptz.data;
	double down_t = 0.0;
	//cout << "pos_in_ptz " << pos_in_ptz << endl;
	if (bullet_speed > 10e-3)
		down_t = _xyz[2] / 100.0 / bullet_speed;
	//cout << "**********************************  "<< endl;
	//cout << "bullet_speed  " << bullet_speed << endl;
	//cout << "down_t  " << down_t;
	double offset_gravity = 0.5 * 9.8 * down_t * down_t * 100;
	//cout << "offset_gravity  " << offset_gravity << endl;
	double xyz[3] = { _xyz[0], _xyz[1] - offset_gravity, _xyz[2] };
	//cout << "xyz[3]  " << xyz[0] << xyz[1] << xyz[2] << endl;
	double alpha = 0.0, theta = 0.0;
	//cout << "offset_y_barrel_ptz  " << offset_y_barrel_ptz << endl;
	//cout << "sqrt(xyz[1] * xyz[1] + xyz[2] * xyz[2])  " << sqrt(xyz[1] * xyz[1] + xyz[2] * xyz[2]) << endl;

	alpha = asin(offset_y_barrel_ptz / sqrt(xyz[1] * xyz[1] + xyz[2] * xyz[2]));
	//cout << "alpha  " << alpha << endl;
	if (xyz[1] < 0){
		cout << "1" << endl;
		theta = atan(-xyz[1] / xyz[2]);
		angle_y = -(alpha + theta);  // camera coordinate
	}
	else if (xyz[1] < offset_y_barrel_ptz){
		cout << "2" << endl;
		theta = atan(xyz[1] / xyz[2]);
		angle_y = -(alpha - theta);  // camera coordinate
	}
	else{
		cout << "3  "<<endl;
		theta = atan(xyz[1] / xyz[2]);
		angle_y = (theta - alpha);   // camera coordinate
	}
	//cout << "theta  " << theta << endl;
	angle_x = atan2(xyz[0], xyz[2]);
	//cout << "angle_x: " << angle_x << "\tangle_y: " << angle_y <<  "\talpha: " << alpha << "\ttheta: " << theta << endl;
	angle_x = angle_x * 180 / 3.1415926;
	angle_y = angle_y * 180 / 3.1415926;
	cout << "angle_x=  " << abs(angle_x) << "angle_y=  " << abs(angle_y) << endl;
}
//将图像坐标系顺序与世界坐标系顺序一一对应起来
void AngleSolver::getTarget2dPoinstion(const cv::RotatedRect & rect, vector<Point2f> & target2d, const cv::Point2f & offset){
	//cout << "**************************" << endl;
	//cout << "**************************" << endl;
	Point2f vertices[4];
	rect.points(vertices);
	//cout << vertices[0] << vertices[1] << vertices[2] << vertices[3] << endl;
	Point2f lu, ld, ru, rd;
	sort(vertices, vertices + 4, [](const Point2f & p1, const Point2f & p2) { return p1.x < p2.x; });
	if (vertices[0].y < vertices[1].y){
		lu = vertices[0];
		ld = vertices[1];
	}
	else{
		lu = vertices[1];
		ld = vertices[0];
	}
	if (vertices[2].y < vertices[3].y)	{
		ru = vertices[2];
		rd = vertices[3];
	}
	else {
		ru = vertices[3];
		rd = vertices[2];
	}

	target2d.clear();
	target2d.push_back(lu + offset);
	target2d.push_back(ru + offset);
	target2d.push_back(rd + offset);
	target2d.push_back(ld + offset);
}

//设置装甲块尺寸
void AngleSolverFactory::setTargetSize(double width, double height, TargetType type){
	if (type == TARGET_RUNE){
		rune_width = width;
		rune_height = height;
	}
	else if (type == TARGET_ARMOR){
		armor_width = width;
		armor_height = height;
	}
	else if (type == TARGET_SAMLL_ATMOR){
		small_armor_width = width;
		small_armor_height = height;
	}
}
//设置工厂类结算器
bool AngleSolverFactory::getAngle(const cv::RotatedRect & rect, TargetType type, double & angle_x, double & angle_y, double bullet_speed, double current_ptz_angle, const cv::Point2f & offset){
	if (slover == NULL){
		std::cerr << "slover not set\n";
		return false;
	}

	double width = 0.0, height = 0.0;
	if (type == TARGET_RUNE){
		width = rune_width;
		height = rune_height;
	}
	else if (type == TARGET_ARMOR){
		width = armor_width;
		height = armor_height;
	}
	else if (type == TARGET_SAMLL_ATMOR){
		width = small_armor_width;
		height = small_armor_height;
	}
	cv::RotatedRect rect_rectifid = rect;
	//AngleSolverFactory::adjustRect2FixedRatio(rect_rectifid, width / height);
	//slover->setTargetSize(width, height);×°¼×½Ó¿Ú
	return slover->getAngle(rect_rectifid, angle_x, angle_y, bullet_speed, current_ptz_angle, offset);
}
