#include "stdafx.h"
#include <opencv2/opencv.hpp>
#include "highgui.h"
#include <iostream>

using namespace std;
using namespace cv;

#define defaultNbSamples 20		//ÿ�����ص����������
#define defaultReqMatches 2		//#minָ��
#define defaultRadius 20		//Sqthere�뾶
#define defaultSubsamplingFactor 16	//�Ӳ�������
#define background 0		//��������
#define foreground 255		//ǰ������
#define confidentInterval 0.11   //the constant for calculate condifence interval

void Initialize(CvMat* pFrameMat, RNG rng);//��ʼ��
void update(CvMat* pFrameMat, CvMat* segMat, RNG rng, int nFrmNum);//����
float max_float(float a, float b);
char* get_Name(int& nFrmNum, char* inName);
