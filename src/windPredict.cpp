/*----------include-----------*/
#include "windPredict.h"
/*----------------------------*/

/*-----------------变量区----------------------*/
Point2f pointSet[pointSetnumeber];/*定义点集,存放目标点*/
int pointNumber = 0;/*配合pointSet[pointSetnumeber]，存放点*/
int runNumber = 0;/*存放目标点的次数，次数达标开始预测*/
float speed;
float acceleratedSpeed;/*速度，加速度*/
float speedSet[speedSetnumber];/*速度集合*/
int speedNumber = 0;/*配合speedSet[speedSetnumber]，存放速度*/
float predictdistance;
Point2f predictPoint;/*定义预测距离和预测点*/
float lastDistance = 0;/*初始化距离*/
int frame;/*帧数*/
int color;/*控制识别颜色，0代表蓝色，1代表红色*/
/*-------------------------------------------------------------------------*/

/********************************************************************************
版权：2023-2024，Climber 
函数名：distance
介绍:计算距离
参数：
作者：袁骁
版本：2023.1.15.01
完成时间：2023.1.15
* ********************************************************************************/
float distance(Point2f lastPoint, Point2f presentPoint){
	float distance;
	distance = sqrt((presentPoint.x - lastPoint.x) * (presentPoint.x - lastPoint.x) + (presentPoint.y - lastPoint.y) * (presentPoint.y - lastPoint.y));
	return distance;
}

/********************************************************************************
版权：2023-2024，Climber 
函数名：predict
介绍: 预测运动轨迹
参数：
作者：袁骁
版本：2023.1.15.01
完成时间：2023.1.15
* ********************************************************************************/
Point2f predict(Mat image,int runNumber,int color){
    Mat midImage;
	/*开始处理图像*/
	clock_t start = clock();
	/*改变大小，提高帧率*/
	resize(image, image, Size(image.cols *1.5, image.rows *1.5)); 
	/*测试效果展示*/
	Mat test;
	image.copyTo(test);
	/*容器，存放分离通道后的图像*/
	vector<Mat> imgChannels;
	split(image, imgChannels);
    if(color==red){
        //红色
	    midImage = imgChannels.at(2) - imgChannels.at(0); 
    }
    else if(color==blue){
        midImage = imgChannels.at(0) - imgChannels.at(2);
    }
    else{
        printf("color error\n");
    }
	/*蓝色*/
	
	Mat binaryImage;
	/*二值化*/
	threshold(midImage, binaryImage, 100, 255, THRESH_BINARY);
	/*形态学闭操作*/
	Mat morphlogyImage;
	Mat element;
	element = getStructuringElement(MORPH_RECT, Size(3, 3));
	morphologyEx(binaryImage, morphlogyImage, MORPH_CLOSE, element);
	/*轮廓发现*/
	vector<vector<Point>> contours;
	vector<Vec4i> hierarchy;
	findContours(morphlogyImage, contours, hierarchy, RETR_TREE, CHAIN_APPROX_NONE);
    if(contours.size()!=0){
        for (int i = 0; i < contours.size(); i++){
            vector<Point>points;   
            double area = contourArea(contours[i]);
            /*面积排除噪声*/
            if (area < 200 || area>10000) 
                continue;
            /*找到有父轮廓的轮廓*/
            if (hierarchy[i][3] < 0 || hierarchy[i][3] >= contours.size()) 
                continue;
            /*找没同级轮廓的*/
            if ((hierarchy[i][0] > 0 && hierarchy[i][0] <= contours.size()) || (hierarchy[i][1] > 0  &&  hierarchy[i][1] <= contours.size())) 
                continue;
            /*得到轮廓*/
            points = contours[i];
            /*计算矩*/
            Moments rect;
            rect = moments(contours[i], false);
            /*计算中心矩:*/
            Point2f rectmid;
            rectmid = Point2f(rect.m10 / rect.m00, rect.m01 / rect.m00);
            /*画出需打部位轮廓*/
            drawContours(test, contours, i, Scalar(0, 255, 255), 2, 8);
            /*画出需打部位中心点*/
            circle(test, rectmid, 1, Scalar(0, 255, 255), -1, 8, 0);
            /*将中心点存入点集*/
            pointSet[pointNumber] = rectmid;
            pointNumber++;
            /*实现新点替换旧点*/
            if (pointNumber == pointSetnumeber) {
                pointNumber = 0;
            } 
        }
    }
    else{
        //
    }
	
		/*预测*/
		if (runNumber > pointSetnumeber) {
			int i = pointNumber - 1;//取最新的点算速度
			int number1 = i;
			if (number1 < 0) {
				number1+= pointSetnumeber;
			}
			int number2 = i - 1;
			if (number2 < 0) {
				number2 += pointSetnumeber;
			}
			int number3 = i - 3;
			if (number3 < 0) {
				number3 += pointSetnumeber;
			}
			int number4 = i - 4;
			if (number4 < 0){
				number4 += pointSetnumeber;
			}
			/*取最近四点，算速度，求加速度*/
			speed = distance(pointSet[number1], pointSet[number2])*frame;
			speedSet[0] = speed;
			speed = distance(pointSet[number3], pointSet[number4]) * frame;
			speedSet[1] = speed;
			acceleratedSpeed  = fabs((speedSet[0]-speedSet[1])*frame);

			/* X = V0T + 1 / 2AT'2，通过距离公式，算预测的打击点距离 */
			predictdistance =4.5* speedSet[0] / frame + 1 / 2 * acceleratedSpeed / frame / frame*18;
			/*防止帧率浮动较大，减小抖动*/
			if (fabs(lastDistance - predictdistance)>2 && lastDistance!=0) {
				if (lastDistance - predictdistance > 0){
					predictdistance = lastDistance-1 ;
				}
				else {
					predictdistance = lastDistance+1 ;
				}
			}
			/*算出预测时x, y需增加值的比值*/
			float xRatio, yRatio;
			xRatio= fabs(pointSet[number1].x - pointSet[number2].x) / distance(pointSet[number1], pointSet[number2]);
			yRatio = fabs(pointSet[number1].y - pointSet[number2].y) / distance(pointSet[number1], pointSet[number2]);
			/*第一象限内  顺  三逆*/
			if (pointSet[number1].x >= pointSet[number2].x && pointSet[number1].y >= pointSet[number2].y){
				predictPoint = Point2f (pointSet[number1].x + predictdistance* xRatio, pointSet[number1].y+ predictdistance * yRatio);
			}
			/*第二象限内  顺  四逆*/
			if (pointSet[number1].x >= pointSet[number2].x && pointSet[number1].y <= pointSet[number2].y) {
				predictPoint = Point2f(pointSet[number1].x + predictdistance * xRatio, pointSet[number1].y - predictdistance * yRatio);
			}
			/*第三象限内  顺  一逆*/
			if (pointSet[number1].x <= pointSet[number2].x && pointSet[number1].y <= pointSet[number2].y) {
				predictPoint = Point2f(pointSet[number1].x - predictdistance * xRatio, pointSet[number1].y - predictdistance * yRatio);
			}
			/*第四象限内  顺  二逆*/
			if (pointSet[number1].x <= pointSet[number2].x && pointSet[number1].y >= pointSet[number2].y) {
				predictPoint = Point2f(pointSet[number1].x - predictdistance * xRatio, pointSet[number1].y + predictdistance * yRatio);
			}
			/*画出预测点*/
			circle(test, predictPoint, 5, Scalar(0,0, 255), -1, 8, 0);
			/*减少预测抖动*/
			lastDistance = predictdistance;    
		}
		imshow("cameraImage", image);
		imshow("test",test);
		clock_t end = clock();
		frame = 1 / (double)(end - start) * CLOCKS_PER_SEC;
		cout << frame <<"帧"<< endl;
		waitKey(5);
        if(runNumber> pointSetnumeber){
            return predictPoint;
        }
        else{
            return Point2f(0,0);
        }
}