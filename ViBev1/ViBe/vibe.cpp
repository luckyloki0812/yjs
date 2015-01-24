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

int nFrmNum = 0;//��¼֡��
int Width;//��¼֡�Ŀ��
int Height;//��¼֡�ĸ߶�
int FrameRate;//��¼��Ƶ֡��

int main(int argc, char* argv[])
{
	IplImage* pFrame = NULL; 
	IplImage* pAfter = NULL; CvMat* pAfterMat = NULL;//����pFrame��Ӧ�ĻҶ�ͼ��
	IplImage* segMap = NULL; CvMat* segMat = NULL;//���洦������Ϣ
	IplImage* dstImg = NULL;
	CvVideoWriter *Seg_result = NULL;

	char filename[100];//�����ͼ��λ�ú�����
	char outfilename[50];//the name of segmentation result video file
	char frameNum[50]="000000";//֡��������ַ�������ʽ���棬Ϊ�˴�һ����֡���ļ�
	char *inName = NULL;
	char *outName = NULL;

	CvSize size1, size2;

	Mat segmentation;//It is used to erode and dilate

	inName = argv[1];
	outName = argv[2];
	size1 = cvSize(Width, Height);
	size2 = cvSize(Width * 2, Height);

	sprintf(outfilename, "%s.avi", outName);

	//��������
	cvNamedWindow("video", CV_WINDOW_AUTOSIZE);

	//����һ�������������
	RNG rng(0xFFFFFFFF);
	//��������
	CvFont font;
	cvInitFont(&font, CV_FONT_HERSHEY_COMPLEX_SMALL, 1, 1, 0, 1, 8);

	//get first file name
	nFrmNum++;
	strcpy(filename, get_Name(nFrmNum, inName));
	//��֡��ȡ��Ƶ�����д���
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
			update(pAfterMat, segMat, rng, nFrmNum);//���±���

			segmentation = segMat;
			//erode(segmentation, segmentation, Mat());
			//dilate(segmentation, segmentation, Mat());
			*segMat = segmentation;

			cvConvert(segMat, segMap);

			//����ԭͼ��Ŀ��ͼ��
			cvSetImageROI(dstImg, cvRect(0, 0, pFrame->width, pFrame->height));//params of cvRect are start_x,start_y,end_x,end_y.
			cvCopy(pFrame, dstImg);
			cvResetImageROI(dstImg);
			//��segMapת������ͨ��ͼ�����pFrame��
			cvCvtColor(segMap, pFrame, CV_GRAY2BGR);
			//��������ͼ��Ŀ��ͼ��
			cvSetImageROI(dstImg, cvRect(pFrame->width, 0, pFrame->width * 2, pFrame->height));
			cvCopy(pFrame, dstImg);
			cvResetImageROI(dstImg);
			//��ʾ��ʾ����
			cvPutText(dstImg, "Origin Video", cvPoint(0, pFrame->height - 5), &font, CV_RGB(255, 255, 0));
			cvPutText(dstImg, "ViBe Video", cvPoint(pFrame->width, pFrame->height - 5), &font, CV_RGB(255, 255, 0));

			//������Ƶ�����
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

