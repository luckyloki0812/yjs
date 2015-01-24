#include "stdafx.h"
#include "function.h"
#include <opencv2/opencv.hpp>
#include "highgui.h"
#include <math.h>
#include <iostream>

using namespace std;
using namespace cv;

static int c_xoff[9] = { -1, 0, 1, -1, 1, -1, 0, 1, 0 };//x的邻居点
static int c_yoff[9] = { -1, 0, 1, -1, 1, -1, 0, 1, 0 };//y的邻居点

float samples[1024][1024][defaultNbSamples + 1];//保存每个像素点的样本值

float max_float(float a, float b)
{
	if (a > b) return a;
	else return b;
}

char* get_Name(int& nFrmNum, char* inName)
{
	int p = 0;
	char filename[100];
	char frameNum[50] = "000000";//帧的序号以字符串的形式保存，为了打开一个个帧的文件

	p = nFrmNum;
	for (int i = 0; i < 5; i++)
	{
		frameNum[i] = p / (int)pow(10, 5 - i) + 48;
		p %= (int)pow(10, 5 - i);
	}
	frameNum[5] = p % 10 + 48;
	sprintf(filename, "%s%s.jpg", inName, frameNum);
	return filename;
}

//初始化
void Initialize(CvMat* pFrameMat, RNG rng){

	//记录随机生成的 行(r) 和 列(c)
	int rand, r, c;

	//对每个像素样本进行初始化
	for (int y = 0; y<pFrameMat->rows; y++){//Height
		for (int x = 0; x<pFrameMat->cols; x++){//Width
			for (int k = 0; k<defaultNbSamples; k++){
				//随机获取像素样本值
				rand = rng.uniform(0, 9);
				r = y + c_yoff[rand]; if (r<0) r = 0; if (r >= pFrameMat->rows) r = pFrameMat->rows - 1;	//行
				c = x + c_xoff[rand]; if (c<0) c = 0; if (c >= pFrameMat->cols) c = pFrameMat->cols - 1;	//列
				//存储像素样本值
				samples[y][x][k] = CV_MAT_ELEM(*pFrameMat, float, r, c);
			}
			samples[y][x][defaultNbSamples] = 0;
		}
	}
}


//更新函数
void update(CvMat* pFrameMat, CvMat* segMat, RNG rng, int nFrmNum){

	for (int y = 0; y<pFrameMat->rows; y++){	//Height
		for (int x = 0; x<pFrameMat->cols; x++){	//Width

			//用于判断一个点是否是背景点,index记录已比较的样本个数，count表示匹配的样本个数
			int count = 0, index = 0; float dist = 0; float interval;
			//
			while ((count<defaultReqMatches) && (index<defaultNbSamples)){
				interval = max_float(confidentInterval*samples[x][y][index], defaultRadius);
				//printf("interval=%f\n", interval);
				dist = CV_MAT_ELEM(*pFrameMat, float, y, x) - samples[y][x][index];
				if (dist<0) dist = -dist;
				if (dist<interval) count++; 
				index++;
			}
			if (count >= defaultReqMatches){

				//判断为背景像素,只有背景点才能被用来传播和更新存储样本值
				samples[y][x][defaultNbSamples] = 0;

				*((float *)CV_MAT_ELEM_PTR(*segMat, y, x)) = background;

				int rand = rng.uniform(0, defaultSubsamplingFactor);
				if (rand == 0){//只用当rand=0时，才更新当前像素的样点集
					rand = rng.uniform(0, defaultNbSamples);//哪一个样点被替换是随机的
					samples[y][x][rand] = CV_MAT_ELEM(*pFrameMat, float, y, x);
				}
				rand = rng.uniform(0, defaultSubsamplingFactor);
				if (rand == 0){//只有当rand=0时，才更新8-邻域内像素的样点集
					int xN, yN;
					rand = rng.uniform(0, 9); yN = y + c_yoff[rand]; if (yN<0) yN = 0; if (yN >= pFrameMat->rows) yN = pFrameMat->rows - 1;
					rand = rng.uniform(0, 9); xN = x + c_xoff[rand]; if (xN<0) xN = 0; if (xN >= pFrameMat->cols) xN = pFrameMat->cols - 1;
					rand = rng.uniform(0, defaultNbSamples);//邻域内哪一个像素的样点集的哪一个样点被更新是随机的
					samples[yN][xN][rand] = CV_MAT_ELEM(*pFrameMat, float, y, x);
				}
			}
			else {
				//判断为前景像素
				*((float *)CV_MAT_ELEM_PTR(*segMat, y, x)) = foreground;

				samples[y][x][defaultNbSamples]++;
				if (samples[y][x][defaultNbSamples]>50){
					int rand = rng.uniform(0, defaultNbSamples);
					if (rand == 0){//这是一个改进方案，当同一个位置的像素值超过50次被检测为前景则用它来更新当前位置的样点集
						rand = rng.uniform(0, defaultNbSamples);
						samples[y][x][rand] = CV_MAT_ELEM(*pFrameMat, float, y, x);
					}
				}
			}

		}
	}

}
