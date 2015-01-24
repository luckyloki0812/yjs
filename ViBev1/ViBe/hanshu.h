#include "stdafx.h"
#include <opencv2/opencv.hpp>
#include "highgui.h"
#include <iostream>

using namespace std;
using namespace cv;

#define defaultNbSamples 20		//每个像素点的样本个数
#define defaultReqMatches 2		//#min指数
#define defaultRadius 20		//Sqthere半径
#define defaultSubsamplingFactor 16	//子采样概率
#define background 0		//背景像素
#define foreground 255		//前景像素
#define confidentInterval 0.11   //the constant for calculate condifence interval

void Initialize(CvMat* pFrameMat, RNG rng);//初始化
void update(CvMat* pFrameMat, CvMat* segMat, RNG rng, int nFrmNum);//更新
float max_float(float a, float b);
char* get_Name(int& nFrmNum, char* inName);
