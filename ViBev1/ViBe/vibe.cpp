// vibe.cpp : Defines the entry point for the console application.
//

// lx_Picture.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <opencv2/opencv.hpp>
#include "highgui.h"
#include "function.h"
#include <iostream>

using namespace std;
using namespace cv;

int nFrmNum = 0;//记录帧数
int Width;//记录帧的宽度
int Height;//记录帧的高度
int FrameRate;//记录视频帧率

int main(int argc, char* argv[])
{
	IplImage* pFrame = NULL; 
	IplImage* pAfter = NULL; CvMat* pAfterMat = NULL;//保存pFrame对应的灰度图像
	IplImage* segMap = NULL; CvMat* segMat = NULL;//保存处理后的信息
	IplImage* dstImg = NULL;
	CvVideoWriter *Seg_result = NULL;

	char filename[100];//保存的图像位置和名称
	char outfilename[50];//the name of segmentation result video file
	char frameNum[50]="000000";//帧的序号以字符串的形式保存，为了打开一个个帧的文件
	char *inName = NULL;
	char *outName = NULL;

	CvSize size1, size2;

	Mat segmentation;//It is used to erode and dilate

	inName = argv[1];
	outName = argv[2];
	size1 = cvSize(Width, Height);
	size2 = cvSize(Width * 2, Height);

	sprintf(outfilename, "%s.avi", outName);

	//创建窗口
	cvNamedWindow("video", CV_WINDOW_AUTOSIZE);

	//创建一个随机数生成器
	RNG rng(0xFFFFFFFF);
	//定义字体
	CvFont font;
	cvInitFont(&font, CV_FONT_HERSHEY_COMPLEX_SMALL, 1, 1, 0, 1, 8);

	//get first file name
	nFrmNum++;
	strcpy(filename, get_Name(nFrmNum, inName));
	//逐帧读取视频并进行处理
	while ((pFrame = cvLoadImage(filename)))
	{
		if (nFrmNum == 1)
		{
			printf("width=%d,height=%d\n", pFrame->width, pFrame->height);
			Width = pFrame->width;
			Height = pFrame->height;
			size1 = cvSize(Width, Height);
			size2 = cvSize(Width * 2, Height);

			Seg_result = cvCreateVideoWriter(outfilename, CV_FOURCC('X', 'V', 'I', 'D'), 25, size2, 1);
		}
		//allocate memory
		segMap = cvCreateImage(size1, IPL_DEPTH_8U, 1);
		segMat = cvCreateMat(pFrame->height, pFrame->width, CV_32FC1);
		pAfter = cvCreateImage(size1, IPL_DEPTH_8U, 1);
		pAfterMat = cvCreateMat(pFrame->height, pFrame->width, CV_32FC1);
		dstImg = cvCreateImage(size2, IPL_DEPTH_8U, 3);

		cvCvtColor(pFrame, pAfter, CV_BGR2GRAY);
		cvConvert(pAfter, pAfterMat);

		if (nFrmNum == 1)
			Initialize(pAfterMat, rng);//initialize the background
		else
		{
			update(pAfterMat, segMat, rng, nFrmNum);//更新背景

			segmentation = segMat;
			//erode(segmentation, segmentation, Mat());
			//dilate(segmentation, segmentation, Mat());
			*segMat = segmentation;

			cvConvert(segMat, segMap);

			//载入原图像到目标图像
			cvSetImageROI(dstImg, cvRect(0, 0, pFrame->width, pFrame->height));//params of cvRect are start_x,start_y,end_x,end_y.
			cvCopy(pFrame, dstImg);
			cvResetImageROI(dstImg);
			//将segMap转换成三通道图像存在pFrame中
			cvCvtColor(segMap, pFrame, CV_GRAY2BGR);
			//载入检测后的图像到目标图像
			cvSetImageROI(dstImg, cvRect(pFrame->width, 0, pFrame->width * 2, pFrame->height));
			cvCopy(pFrame, dstImg);
			cvResetImageROI(dstImg);
			//显示提示文字
			cvPutText(dstImg, "Origin Video", cvPoint(0, pFrame->height - 5), &font, CV_RGB(255, 255, 0));
			cvPutText(dstImg, "ViBe Video", cvPoint(pFrame->width, pFrame->height - 5), &font, CV_RGB(255, 255, 0));

			//保存视频和输出
			cvWriteFrame(Seg_result, dstImg);

			cvShowImage("video", dstImg);
		}
			
		//get next file name
		nFrmNum++;
		strcpy(filename, get_Name(nFrmNum, inName));

		//release memory
		cvReleaseImage(&pFrame);
		cvReleaseImage(&pAfter); cvReleaseMat(&pAfterMat);
		cvReleaseImage(&segMap); cvReleaseMat(&segMat);
		cvReleaseImage(&dstImg);

		if (cvWaitKey(5) >= 0) break;
	}
	
	cvReleaseVideoWriter(&Seg_result);
	cvDestroyWindow("video");

	return 0;
}

