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
		trackBuff[i].init(100);
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
			//trackBuff[k].updateAFrame(x,y,frameidx);
			trackBuff[k].updateAFrame(ptmp);
		}
	}

	return true;
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

	int * aptr=strk.getPtr(0),*bptr=aptr;
	int xa=(*aptr),ya=(*(aptr+1)),xb=*(strk.cur_frame_ptr),yb=*(strk.cur_frame_ptr+1);
	//std::cout<<xa<<"_"<<ya<<"_"<<xb<o<"_"<<yb<<"|";
	double displc=sqrt( pow(xb-xa, 2.0) + pow(yb-ya, 2.0));
	for(int posi=0;posi<strk.len;posi++)
	{
		bptr=strk.getPtr(posi);
		xb=(*bptr),yb=(*(bptr+1));
		dtmp = sqrt( pow(xb-xa, 2.0) + pow(yb-ya, 2.0));
		totlen+=dtmp;
		if (dtmp > maxdist&&posi>=startidx) maxdist = dtmp;
		xa=xb,ya=yb;
	}
	
	//if(strk.len>20&&totlen*0.5>displc){isTrkValid = false;std::cout<<totlen<<"_"<<displc<<"|";}
	if (maxdist < 1.4&&strk.len>30){isTrkValid = false;}
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
			trackBuff[k].updateAFrame(ptmp);

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
			trackBuff[k].updateAFrame(ptmp);
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
	delete x_idx;
	delete y_idx;
}

void KLTtracker::drawStuff(unsigned char* cfarmeptr)
{

	for (int i = 0; i < nFeatures; ++i)
	{
		if (trackBuff[i].len > 5)
		{
			for (int j = 1; j <trackBuff[i].len; ++j)
			{
				int *aptr=trackBuff[i].getPtr(j-1);
				int *bptr=trackBuff[i].getPtr(j);
				drawline(cfarmeptr,*aptr,*(aptr+1),*bptr,*(bptr+1),_line_colos[i%6]);
			}
		}
	}
}

int checkidx=720, checkx=0,checky=0,checkt=0;
vector<int> idxvec;
void KLTtracker::drawStuffScale(unsigned char* cfarmeptr,int w,int h)
{
	drawW=w,drawH=h;
	double scaleW=((double)drawW)/((double)frame_width);
	double scaleH=((double)drawH)/((double)frame_height);
	Mat	 frameMat(drawH, drawW, CV_8UC3, cfarmeptr);
	int xa,xb,ya,yb,fval=0;
	/*
	if(checkt==0&&trackBuff[checkidx].len>10)
	{
		checkx=*(trackBuff[checkidx].cur_frame_ptr);
		checky=*(trackBuff[checkidx].cur_frame_ptr+1);
		checkt++;
		idxvec=vector<int>(0);
		idxvec.push_back(checkidx);
	}
	*/
	for (int i = 0; i < nFeatures; ++i)
	{
		/*
		int sx=*(trackBuff[i].cur_frame_ptr);
		int sy=*(trackBuff[i].cur_frame_ptr+1);
		bool wantcheck=false;
		for(int ii=0;ii<idxvec.size();ii++)
		{
			if(i==idxvec[ii])
			{
				wantcheck=true;
			}
		}
		if(!wantcheck&&sx==checkx&&sy==checky)
		{
			idxvec.push_back(i);
			wantcheck=true;
		}
		*/
		if (trackBuff[i].len > 5 )//&& isTracking[i])
		{
			double maxdist = .0, dtmp = .0,totlen=.0;	
			int * aptr=trackBuff[i].getPtr(0),*bptr=aptr;
			int xa=(*aptr),ya=(*(aptr+1)),xb=*(trackBuff[i].cur_frame_ptr),yb=*(trackBuff[i].cur_frame_ptr+1);
			double displc=sqrt( pow(xb-xa, 2.0) + pow(yb-ya, 2.0));
			for (int j = 1; j <trackBuff[i].len; ++j)
			{
				aptr=trackBuff[i].getPtr(j-1);
				bptr=trackBuff[i].getPtr(j);
				xa=(*aptr)*scaleW,xb=(*bptr)*scaleW,ya=(*(aptr+1))*scaleH,yb=(*(bptr+1))*scaleH;
				//drawline(cfarmeptr,xa,ya,xb,yb,_line_colos[i%6]);
				dtmp= sqrt( pow((*bptr)-(*aptr), 2.0) + pow((*(bptr+1))-(*(aptr+1)), 2.0));
				totlen+=dtmp;
				if(dtmp>maxdist)maxdist=dtmp;
			}
			unsigned char rgb[3]={_line_colos[i%6][0],_line_colos[i%6][1],_line_colos[i%6][2]};
			if(trackBuff[i].len>20 && totlen*0.5>displc)
			{
			}
			else
			{
				rgb[0]=((float)rgb[0]/3);
				rgb[1]=((float)rgb[1]/3);
				rgb[2]=((float)rgb[2]/3);
			}
			for (int j = 1; j <trackBuff[i].len; ++j)
			{
				aptr=trackBuff[i].getPtr(j-1);
				bptr=trackBuff[i].getPtr(j);
				xa=(*aptr)*scaleW,xb=(*bptr)*scaleW,ya=(*(aptr+1))*scaleH,yb=(*(bptr+1))*scaleH;
				drawline(cfarmeptr,xa,ya,xb,yb,rgb);
			}
			sprintf(strbuff,"%.1f,%.1f",displc,totlen);
			putText(frameMat,strbuff, Point(xb,yb), FONT_HERSHEY_SIMPLEX, 0.4, Scalar(rgb[0],rgb[1],rgb[2]));
		}
	}
}
int KLTtracker::endTraking()
{
  return 0;
}
