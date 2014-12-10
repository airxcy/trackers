#ifndef KLTTRACKER_H
#define KLTTRACKER_H

#include "itf/trackers/tracker.h"
#include "itf/trackers/klt_c/klt.h"
#include "itf/trackers/klt_c/pnmio.h"
#include "itf/trackers/klt_c/trk.h"
#include <math.h>

#include <opencv2/opencv.hpp>
using namespace cv;

using namespace std;
class KLTtracker : public tracker
{
public:
	KLT_TrackingContext tc;
	KLT_FeatureList fl;
	KLT_FeatureTable ft;

	int *isTracking, *trkIndex;
	int bgMod;

	int frameidx;
	int nFeatures; /*** get frature number ***/
	int FOREGROUND_THRESHOLD;
	int curMaxTrk;
	int drawW,drawH;
	unsigned char* preframedata,* bgdata,*curframedata;

	std::vector<TrackBuff> trackBuff;

	int init(int bsize,int w,int h);
	int selfinit(unsigned char* framedata);
	int initBG(int mode,unsigned char* bgptr=NULL);

	int updateAframe(unsigned char* framedata);
	void updateFGMask(unsigned char* bgptr);
	
	int checkFG(int x,int y);
	bool checkTrackMoving(TrackBuff &strk);

	void drawStuff(unsigned char* cfarmeptr);
	void drawStuffScale(unsigned char* cfarmeptr,int w,int h);
	void drawline(unsigned char* cfarmeptr,int x1,int y1,int x2, int y2,unsigned char* rg);
	int endTraking();
};
#endif
