#include "itf/trackers/klttracker.h"


#define REPLACE
#define MAX_FEATURE_NUMBER  5000
unsigned char _line_colos[6][3] =
{
	{0,255,0},
	{0,0,255},
	{255,255, 0},
	{255,0,255},
	{0,255,255},
	{255,0,0},
};
char strbuff[100];

int KLTtracker::init(int bsize,int w,int h)
{
	
	nFeatures = 1000;
	FOREGROUND_THRESHOLD = 0;
	isTracking = (int*)malloc(sizeof(int) * nFeatures);
	trkIndex = (int*)malloc(sizeof(int) * nFeatures);
	trackBuff=std::vector<TrackBuff>(nFeatures);
	for (int i=0;i<nFeatures;i++)
	{
		isTracking[i]=0;
		trkIndex[i]=0;
		//trackBuff[i].init(3,10);
		trackBuff[i].init(1,100);
	}
	frame_width = w;
	frame_height = h;
	frameidx=0;
	tc = KLTCreateTrackingContext();
	fl = KLTCreateFeatureList(nFeatures);
	tc->sequentialMode = TRUE;
	tc->writeInternalImages = FALSE;
	tc->affineConsistencyCheck = -1;  /* set this to 2 to turn on affine consistency check */
	//tc->min_determinant=0.01;
	tc->window_width=7;
	tc->window_height=7;
	//tc->lighting_insensitive=TRUE;
	KLTSetVerbosity(0);
	drawW=frame_width,drawH=frame_height;
	bgMod=0;
	delay=0;
	gt_inited = false;
	return 1;
}
int KLTtracker::selfinit(unsigned char* framedata)
{
	curframedata=framedata;
	bgdata = new unsigned char[frame_width*frame_height];
	memset(bgdata,0,frame_width*frame_height);
	preframedata = new unsigned char[frame_width*frame_height];
	memcpy(preframedata,framedata,frame_width*frame_height);
	KLTSelectGoodFeatures(tc, framedata, frame_width, frame_height, fl);
	KLTUpdateTCBorder(tc);
	for (int k = 0; k < nFeatures; k++)
	{
		if(true)// (checkFG( x, y))
		{
			int x = (int) (fl->feature[k]->x+0.5);
			int y = (int) (fl->feature[k]->y+0.5);
			int val= (int) (fl->feature[k]->val);
			int ptmp[3]={x,y,val};

			pttmp.x = (int)(fl->feature[k]->x + 0.5);
			pttmp.y = (int)(fl->feature[k]->y + 0.5);
			pttmp.t = (int)(frameidx + 0.5);
			//trackBuff[k].updateAFrame(x,y,frameidx);
			trackBuff[k].updateAFrame(&pttmp);
		}
	}
	return true;
}

int KLTtracker::initGT(string & gtfilename)
{
	std::cout << gtfilename << std::endl;
	if (gtfilename.length()>1)
	{
		//goodFeaturesToTrack(gray, points[0], MAX_COUNT, 0.01, 0, Mat(), 10, 0, 0.04);
		char strbuf[100];
		int x, y;
		int pos[2], bb[4];
		std::ifstream txtfile(gtfilename);
		if (txtfile.is_open())
		{
			txtfile.getline(strbuf, 100);
			bbw = 40, bbh = 100;
			sscanf_s(strbuf, "%d", &gt_N);
			std::cout << gtfilename << "gt:" << gt_N << std::endl;
			targetLoc.init(2, gt_N);
			targetBB.init(4, gt_N);
			for (int gt_i = 0; gt_i < gt_N; gt_i++)
			{
				txtfile.getline(strbuf, 100);
				sscanf_s(strbuf, "%d,%d", pos, pos + 1);
				std::cout << pos[0] <<","<<pos[1]<< std::endl;
				targetLoc.updateAFrame(pos);
				bb[0] = pos[0] - bbw / 2, bb[1] = pos[1] - bbh / 10, bb[2] = pos[0] + bbw / 2, bb[3] = pos[1] + bbh * 9 / 10;
				targetBB.updateAFrame(bb);
			}
			gt_inited = true;
		}
	}
	return 1;
}

int KLTtracker::initBG(int mode,unsigned char* bgptr)
{
	bgMod=mode;
	if(mode==1&&bgptr!=NULL)
	{
		bgdata=bgptr;
	}
	return true;
}
int KLTtracker::checkFG(int x,int y)
{
	if(bgMod==1)
	{
		return (abs(curframedata[y*frame_width+x]-bgdata[y*frame_width+x])>=FOREGROUND_THRESHOLD);
	}
	else if(bgMod==2)
	{
		return (int)bgdata[y*frame_width+x];
	}
	else 
		return true;
}
void KLTtracker::updateFGMask(unsigned char* bgptr)
{
	bgdata=bgptr;
}

bool KLTtracker::checkTrackMoving(TrackBuff &strk)
{
	bool isTrkValid = true;
	int Movelen=15,minlen=5,startidx=max(strk.len-Movelen,0);
	double maxdist = .0, dtmp = .0,totlen=.0;

	TrkPts* aptr = strk.getPtr(0), *bptr = aptr;
	//int xa=(*aptr),ya=(*(aptr+1)),xb=*(strk.cur_frame_ptr),yb=*(strk.cur_frame_ptr+1);
	int xa=aptr->x,ya=aptr->y,xb=strk.cur_frame_ptr->x,yb=strk.cur_frame_ptr->y;
	double displc=sqrt( pow(xb-xa, 2.0) + pow(yb-ya, 2.0));
	for(int posi=0;posi<strk.len;posi++)
	{
		bptr=strk.getPtr(posi);
		xb=bptr->x,yb=bptr->y;
		dtmp = sqrt( pow(xb-xa, 2.0) + pow(yb-ya, 2.0));
		totlen+=dtmp;
		if (dtmp > maxdist&&posi>=startidx) maxdist = dtmp;
		xa=xb,ya=yb;
	}
	
	//if(strk.len>20&&totlen*0.5>displc){isTrkValid = false;std::cout<<totlen<<"_"<<displc<<"|";}
	if (maxdist < 1.4 && strk.len>30){isTrkValid = false;}
	if (maxdist <=0.1 && strk.len>=minlen){isTrkValid = false;}

	return isTrkValid;
}

int KLTtracker::updateAframe(unsigned char* framedata)
{
	frameidx++;
	curframedata=framedata;
	KLTTrackFeatures(tc, preframedata, framedata, frame_width, frame_height, fl);
	KLTReplaceLostFeatures(tc, framedata, frame_width, frame_height, fl);
	for (int k = 0; k < nFeatures; k++)
	{
		if ( fl->feature[k]->val == 0)
		{
			int x = (int) (fl->feature[k]->x+0.5);
			int y = (int) (fl->feature[k]->y+0.5);
			int val= (int) (fl->feature[k]->val);
			int ptmp[3]={x,y,val};
			//trackBuff[k].updateAFrame(x,y,frameidx);
			pttmp.x = (int)(fl->feature[k]->x + 0.5);
			pttmp.y = (int)(fl->feature[k]->y + 0.5);
			pttmp.t = (int)(frameidx + 0.5);
			//trackBuff[k].updateAFrame(x,y,frameidx);
			trackBuff[k].updateAFrame(&pttmp);

			bool isMoving=checkTrackMoving(trackBuff[k]);
			if (!isMoving) 
			{
				isTracking[k]=0;
				fl->feature[k]->val=-2;
			}
		}
		if ( fl->feature[k]->val > 0)
		{
			trackBuff[k].clear();
			int x = (int) (fl->feature[k]->x);
			int y = (int) (fl->feature[k]->y);
			int val= (int) (fl->feature[k]->val);
			int ptmp[3]={x,y,val};

			//trackBuff[k].updateAFrame(x,y,frameidx);
			pttmp.x = (int)(fl->feature[k]->x + 0.5);
			pttmp.y = (int)(fl->feature[k]->y + 0.5);
			pttmp.t = (int)(frameidx + 0.5);
			//trackBuff[k].updateAFrame(x,y,frameidx);
			trackBuff[k].updateAFrame(&pttmp);
			isTracking[k]=1;
		}
		if ( fl->feature[k]->val < 0)
		{
			trackBuff[k].clear();
		}
	}
	return 1;
}

void KLTtracker::drawline(unsigned char* cfarmeptr,int x1,int y1,int x2, int y2,unsigned char* rgb)
{
	int A[2]={x1,y1};
	int B[2]={x2,y2};
	int x,y,offset;
	int* x_idx=NULL, *y_idx=NULL;
	//std::cout << x1 << "," << y1 << "," << x2 << "," << y2 << std::endl;
	int len=getLineIdx(&x_idx,&y_idx,A,B);
	for(int i=0;i<len;i++)
	{
		x=x_idx[i];
		y=y_idx[i];
		offset=(y*drawW+x);
		//if(x>=0&&y>=0&&x<frame_width&&y<frame_height)
		//{
		cfarmeptr[offset*3]=rgb[0];
		cfarmeptr[offset*3+1]=rgb[1];
		cfarmeptr[offset*3+2]=rgb[2];
		//}
	}
	free( x_idx);
	free(y_idx);
}

void KLTtracker::drawStuff(unsigned char* cfarmeptr)
{

	for (int i = 0; i < nFeatures; ++i)
	{
		if (trackBuff[i].len > 5)
		{
			for (int j = 1; j <trackBuff[i].len; ++j)
			{
				TrkPts *aptr = trackBuff[i].getPtr(j - 1);
				TrkPts *bptr = trackBuff[i].getPtr(j);
				drawline(cfarmeptr,aptr->x,aptr->y,bptr->x,bptr->y,_line_colos[i%6]);
			}
		}
	}
}

int checkidx=720, checkx=0,checky=0,checkt=0;
vector<int> idxvec;
bool KLTtracker::checkCurve(TrackBuff &strk)
{
	double maxdist = .0, dtmp = .0, totlen = .0;
	TrkPts * aptr = strk.getPtr(0), *bptr = aptr;
	int xa = aptr->x, ya = aptr->y, xb = strk.cur_frame_ptr->x, yb = strk.cur_frame_ptr->y;
	double displc = sqrt(pow(xb - xa, 2.0) + pow(yb - ya, 2.0));
	for (int j = 1; j <strk.len; ++j)
	{
		aptr = strk.getPtr(j - 1);
		bptr = strk.getPtr(j);
		dtmp = sqrt(pow(bptr->x - aptr->x, 2.0) + pow(bptr->y - aptr->y, 2.0));
		totlen += dtmp;
		if (dtmp>maxdist)maxdist = dtmp;
	}
	sprintf(strbuff, "%.1f,%.1f\0", displc, totlen);
	return (strk.len > 20 && totlen*0.5 > displc);
}
void KLTtracker::drawStuffScale(unsigned char* cfarmeptr,int w,int h)
{
	drawW=w,drawH=h;
	scaleW=((double)drawW)/((double)frame_width);
	scaleH=((double)drawH)/((double)frame_height);
	Mat	 frameMat(drawH, drawW, CV_8UC3, cfarmeptr);
	TrkPts * aptr, *bptr;
	int xa, ya, xb, yb;
	for (int i = 0; i < nFeatures; ++i)
	{
		if (trackBuff[i].len > 5 )//&& isTracking[i])
		{

			unsigned char rgb[3]={_line_colos[i%6][0],_line_colos[i%6][1],_line_colos[i%6][2]};
			//if(trackBuff[i].len>20 && totlen*0.5>displc)
			if(!checkCurve(trackBuff[i]))
			{
				rgb[0]=((float)rgb[0]/3);
				rgb[1]=((float)rgb[1]/3);
				rgb[2]=((float)rgb[2]/3);
			}
			for (int j = 1; j <trackBuff[i].len; ++j)
			{
				aptr=trackBuff[i].getPtr(j-1);
				bptr=trackBuff[i].getPtr(j);
				xa=aptr->x*scaleW,xb=bptr->x*scaleW,ya=aptr->y*scaleH,yb=bptr->y*scaleH;
				drawline(cfarmeptr,xa,ya,xb,yb,rgb);
			}
			putText(frameMat,strbuff, Point(xb,yb), FONT_HERSHEY_SIMPLEX, 0.4, Scalar(rgb[0],rgb[1],rgb[2]));
		}
	}
	drawBox(cfarmeptr, w, h);
}
void KLTtracker::drawBox(unsigned char* cfarmeptr, int w, int h)
{
	if (gt_inited)
	{
		Mat	 frameMat(drawH, drawW, CV_8UC3, cfarmeptr);
		for (int i = 0; i < gt_N; ++i)
		{
			int* bb = targetBB.getPtr(i);
			int left = bb[0] * scaleW, top = bb[1] * scaleH, right = bb[2] * scaleW, bottom = bb[3] * scaleH;
			unsigned char rgb[3] = { _line_colos[i % 6][0], _line_colos[i % 6][1], _line_colos[i % 6][2] };
			for (int x = left; x < right; x++)
				for (int y = top; y < bottom; y++)
				{
					if (x >= 0 && x < drawW&&y >= 0 && y < drawH)
					{
						int offset = (y*drawW + x);
						cfarmeptr[offset * 3] = (cfarmeptr[offset * 3] * 225 + rgb[0]*30) / 255;
						cfarmeptr[offset * 3 + 1] = (cfarmeptr[offset * 3 + 1] * 225 + rgb[1]*30) / 255;
						cfarmeptr[offset * 3 + 2] = (cfarmeptr[offset * 3 + 2] * 225 + rgb[2]*30) / 255;
					}
				}
		}
		waitKey(0);
	}
	
}
int KLTtracker::endTraking()
{
  return 0;
}
