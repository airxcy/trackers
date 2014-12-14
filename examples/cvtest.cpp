#include "itf/trackers/klttracker.h"
#include "itf/trackers/buffers.h"
#include <opencv2/opencv.hpp>
#include <fstream>
#include <time.h> 
using namespace cv;
//#include "utils.h"

/*
bool readFrame(Mat& frame)
{
	if(isVid)
	{
		vid_cpt.grab();
		return vid_cpt.retrieve( frame, 3);
	}
	else
	{
		sprintf(strbuff,fname_str.c_str(),frameidx);
		frame = imread(strbuff,1);
		frameidx++;
		//while(frame.empty())
		//{
		//	sprintf(strbuff,fname_str.c_str(),frameidx);
		//	frame = imread(strbuff,1);
		//	frameidx++;
		//}
		return !frame.empty();
	}
}
*/
int main(int argc, char** argv )
//int APIENTRY WinMain( HINSTANCE hCurrentInst, HINSTANCE hPreviousInst, LPSTR lpszCmdLine, int nCmdShow)
{
	namedWindow("result");
while(1)
{
	std::string vidfname = argv[1];
	std::string gtdir = "", gtbasedir("C:\\Users\\xcy\\Documents\\CVProject\\data\\label_company\\labels\\"), basedirsufix("txt\\");
	std::string vidid = vidfname.substr(vidfname.length() - 10, 6);
	gtdir = gtbasedir + vidid + basedirsufix;
	int bgmod = stoi(argv[2]);
	VideoCapture cap;
	int frameidx=0;
	char fnamestr[100];
	std::cout<<vidfname<<std::endl;
	cap.open(vidfname);
    if( !cap.isOpened() )
    {
        std::cout << "Could not initialize capturing ...\n";
        return 0;
    }
	Mat frame, gray,fgMask,fgMasktmp;
	cap >> frame;
	
	int vidWidth=frame.size[1],vidHeight=frame.size[0],elem_size=frame.elemSize(),vidbuffLen = 100;
	int dispScale=2;
	clock_t t;
	float fps=0;

	FrameBuff framebuff,graybuff;
	KLTtracker* tracker=new KLTtracker();
	
	framebuff.init(elem_size,vidWidth*dispScale,vidHeight*dispScale,vidbuffLen);
	//graybuff.init(8,vidWidth,vidHeight,vidbuffLen);
	tracker->init(8,vidWidth,vidHeight);

	tracker->initBG(bgmod);
	BackgroundSubtractorMOG2 pMOG2 = BackgroundSubtractorMOG2();
	pMOG2.set("fTau",0);
	Mat white = Mat::ones(vidHeight, vidWidth, CV_8U);
	white=white*255;

	int erosion_size=0,dialate_size=3;
	Mat element = getStructuringElement( MORPH_RECT,Size( 2*erosion_size + 1, 2*erosion_size+1 ),Point( erosion_size, erosion_size ) );
 	Mat element2 = getStructuringElement( MORPH_RECT,Size( 2*dialate_size + 1, 2*dialate_size+1 ),Point( dialate_size, dialate_size ) );

	cap >> frame;
	cvtColor(frame,gray,CV_BGR2GRAY);
	tracker->selfinit(gray.data);
	int padx=tracker->tc->borderx,pady=tracker->tc->bordery; 
	std::cout <<"padx:"<<padx<<"pady:"<<pady<<std::endl;
	t = clock();
	Mat frameScale(vidHeight*dispScale,vidWidth*dispScale,CV_8UC3);
	while(~frame.empty())
	{
		std::cout << frameidx << std::endl;
		//cap.read(frame);
		if ((frameidx - 50) % 1500 == 0)
		{
			sprintf(fnamestr, "%06d.txt", frameidx);
			std::string gtfname = gtdir + fnamestr;
			tracker->initGT(gtfname);
		}
		cvtColor(frame,gray,CV_BGR2GRAY);
		if(bgmod==2)
		{
			pMOG2(gray,fgMask);
			//imshow("mask",fgMask);
			threshold(fgMask, fgMask, 1, 255,THRESH_BINARY);
			//imshow("mask2",fgMask);
			//erode(fgMask, fgMask, element);
			//dilate( fgMask, fgMask, element2);
			tracker->updateFGMask(fgMask.data);
			imshow("mask",fgMask);
		}
		tracker->updateAframe(gray.data);
		resize(frame*0.5, frameScale,frameScale.size());
		tracker->drawStuffScale(frameScale.data,vidWidth*dispScale,vidHeight*dispScale);
		framebuff.updateAFrame(frameScale.data);
		if(frameidx>100)
		{
			t = clock() - t;
			float elapsed = ((float)t)/CLOCKS_PER_SEC;
			fps =frameidx/elapsed;
			std::cout<<"fps:"<<fps<<std::endl;
			frameidx=0;
			t=clock();
		}
		imshow("result",frameScale);
		
		waitKey(1);
		frameidx++;
		cap >> frame;
	}
}
}