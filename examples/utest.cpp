#include "itf/trackers/klttracker.h"
#include "itf/trackers/buffers.h"
#include <opencv2/opencv.hpp>
#include <fstream>
using namespace cv;
using namespace std;
int main()
{
	KLT_Feature f=(KLT_Feature)malloc(sizeof(KLT_FeatureListRec));
	int* i=(int*)f;
	i[0]=1;
	i[1]=2;
	i[3]=3;
	/*
	f->x=1;
	f->y=2;
	f->val=3;
	*/
	std::cout<<f->x<<","<<f->y<<","<<f->val<<std::endl;
	std::cout<<sizeof(KLT_FeatureListRec)/sizeof(int)<<std::endl;
	std::cout<<sizeof(KLT_Feature)/sizeof(int)<<std::endl;
    std::cout<<sizeof(KLT_FeatureRec)/sizeof(int)<<std::endl;
    /*
	KLT_FeatureList fl;
	fl->nFeatures=3;
	fl->feature=(KLT_Feature)malloc(sizeof(KLT_Feature)*3);
	*/
	return 0;
}