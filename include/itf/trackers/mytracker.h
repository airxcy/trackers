#ifndef MYTRACKER_H
#define MYTRACKER_H
#include "tracker.h"
#include <opencv2/opencv.hpp>
//#include "itf/common.hpp"
#include <iostream>
#include <fstream>
using namespace cv;

class MyTrackerKLT:public tracker
{
public:
	float p_width,p_height;
	TermCriteria termcrit;
	Size subPixWinSize,winSize;
    Mat  gray, prevGray, image,image2;
	Mat points0,points1,rel_pos,rel_weight,gtPoints,gtRegion, rel_pos2;
	vector<uchar> status;
    vector<float> err;

	int init(int bsize,int w,int h);
	int updateAframe(unsigned char* framedata);
	int initTargetsLoc(string gtfilename);


	int inregion(Vec2i fp, Vec4f region);
	void getCannyPoints(Mat & outPoints,Mat& weights,Mat& relPos,Mat& gray,Mat& gtPoints,Mat& gtRegion);
	void getGoodPoints(Mat & outPoints,Mat& weights,Mat& relPos,Mat& gray,Mat& gtPoints,Mat& gtRegion);
	//int track(Mat frame,string gtfilename);
};
#endif
