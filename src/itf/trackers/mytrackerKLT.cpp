#include"itf/trackers/mytracker.h"
char errstrbuff[500];
Point2f point;
bool addRemovePt = false;
void cv_debug(string errstrbuff)
{
	Mat errormat(60,300,CV_8UC3);
	putText(errormat,errstrbuff,Point(0,30),FONT_HERSHEY_SIMPLEX,0.5,Scalar(0,0,255));
	imshow("debug",errormat);
	waitKey(0);
}
int MyTrackerKLT::init(int bsize,int w,int h)
{
	elem_size=bsize, frame_width=w, frame_height=h;
    termcrit=TermCriteria(CV_TERMCRIT_ITER|CV_TERMCRIT_EPS, 20, 0.03);
    subPixWinSize=Size(10,10), winSize=Size(31,31);
	p_width=30,p_height=50;
	gt_N=0;
	gt_inited=false;
	return 1;
}
int MyTrackerKLT::inregion(Vec2i fp, Vec4f region){	return (fp[0]>region[0]&&fp[0]<region[2]&&fp[1]>region[1]&&fp[1]<region[3])?1:0 ;}
void MyTrackerKLT::getCannyPoints(Mat & outPoints,Mat& weights,Mat& relPos,Mat& gray,Mat& gtPoints,Mat& gtRegion)
{
	Mat cannymap,edgemat;
	int lowThreshold=20,ratio=3,kernel_size=3;
	int gt_N=gtPoints.size[0];
	Canny( gray, cannymap, lowThreshold, lowThreshold*ratio, kernel_size );
	int fp_N=countNonZero(cannymap);
	findNonZero(cannymap,edgemat);
	// scan for gt related feature points
	//vector<Vec2i> rel(0);
	vector<int> rel(0);
	//rel.reserve(fp_N);
	Mat gtid = Mat::zeros(fp_N,gt_N,CV_8U);
	for(int fp_i=0;fp_i<fp_N;fp_i++)
	{
		int isRelated=0;
		int* p=((int *)edgemat.data)+fp_i*2;
		for(int gt_i=0;gt_i<gt_N;gt_i++)
		{
			uchar inval= inregion(edgemat.at<Vec2i>(fp_i),gtRegion.at<Vec4f>(gt_i));
			isRelated += inval;
			gtid.at<uchar>(fp_i,gt_i)=inval;
		}
		if(isRelated>0)
			//rel.push_back(edgemat.at<Vec2i>(fp_i));
			rel.push_back(fp_i);
	}

	outPoints = Mat(rel.size(),1,CV_32FC2);
	weights = Mat(rel.size(),gt_N,CV_32S);
	relPos = Mat(rel.size(),gt_N,CV_32FC2);
	for(int i=0;i<rel.size();i++)
	{
		Vec2f p=edgemat.at<Vec2i>(rel[i]);
		outPoints.at<Vec2f>(i)=p;
		for(int j=0;j<gt_N;j++)
		{
			if(gtid.at<uchar>(rel[i],j)>0)
			{
				weights.at<int>(i,j)=1;
				relPos.at<Vec2f>(i,j)=gtPoints.at<Vec2f>(j)-p;
			}
			else
			{
				weights.at<int>(i,j)=0;
			}
		}
		/*
		Point2i  p1=Point2i(rel[i]);
		if(p1.x>=0&&p1.x<gray.size[1]&&p1.y>=0&&p1.y<gray.size[0])
		{
			gray.at<uchar>(p1)=255;
		}
		*/
	}
	//imshow("canny",gray);
	//waitKey(0);
}
void MyTrackerKLT::getGoodPoints(Mat & outPoints,Mat& weights,Mat& relPos,Mat& gray,Mat& gtPoints,Mat& gtRegion)
{
	Mat goodPoints;
	int MAX_COUNT=50000;
	int gt_N=gtPoints.size[0];
	goodFeaturesToTrack(gray, goodPoints, MAX_COUNT, 0.00001, 0, Mat(), 3, 0, 0.04);
	int fp_N=goodPoints.size[0];
	// scan for gt related feature points
	//vector<Vec2i> rel(0);
	vector<int> rel(0);
	//rel.reserve(fp_N);
	Mat gtid = Mat::zeros(fp_N,gt_N,CV_8U);
	for(int fp_i=0;fp_i<fp_N;fp_i++)
	{
		int isRelated=0;
		for(int gt_i=0;gt_i<gt_N;gt_i++)
		{
			uchar inval= inregion(goodPoints.at<Vec2f>(fp_i),gtRegion.at<Vec4f>(gt_i));
			isRelated += inval;
			gtid.at<uchar>(fp_i,gt_i)=inval;
		}
		if(isRelated>0)
			//rel.push_back(edgemat.at<Vec2i>(fp_i));
			rel.push_back(fp_i);
	}

	outPoints = Mat(rel.size(),1,CV_32FC2);
	weights = Mat(rel.size(),gt_N,CV_32S);
	relPos = Mat(rel.size(),gt_N,CV_32FC2);
	for(int i=0;i<rel.size();i++)
	{
		Vec2f p=goodPoints.at<Vec2f>(rel[i]);
		outPoints.at<Vec2f>(i)=p;
		for(int j=0;j<gt_N;j++)
		{
			if(gtid.at<uchar>(rel[i],j)>0)
			{
				weights.at<int>(i,j)=1;
				relPos.at<Vec2f>(i,j)=gtPoints.at<Vec2f>(j)-p;
			}
			else
			{
				weights.at<int>(i,j)=0;
			}
		}
		/*
		Point2i  p1=Point2i(rel[i]);
		if(p1.x>=0&&p1.x<gray.size[1]&&p1.y>=0&&p1.y<gray.size[0])
		{
			gray.at<uchar>(p1)=255;
		}
		*/
	}
	//imshow("canny",gray);
	//waitKey(0);
}

int MyTrackerKLT::initTargetsLoc(string gtfilename)
{
	if(gtfilename.length()>1)
	{
		//goodFeaturesToTrack(gray, points[0], MAX_COUNT, 0.01, 0, Mat(), 10, 0, 0.04);
		char strbuf[100];
		int x,y;
		std::ifstream txtfile(gtfilename);
		txtfile.getline(strbuf,100);

		//sscanf_s(strbuf,"%d",&gt_N);
		std::cout <<gtfilename<<",,"<<gtPoints.size[0]<<std::endl;
		gtPoints = Mat(gt_N,1,CV_32FC2),gtRegion=Mat(gt_N,1,CV_32FC4);
		for (int gt_i=0;gt_i<gt_N;gt_i++)
		{
			txtfile.getline(strbuf,100);
			//sscanf_s(strbuf,"%d,%d",&x,&y);
			gtPoints.at<Vec2f>(gt_i)=Vec2f(x,y);
			float left=x-p_width/2,top=y-p_height/10,right=x+p_width/2,bottom=y+p_height*9/10;
			gtRegion.at<Vec4f>(gt_i)=Vec4f(left,top,right,bottom);
			circle( image, Point(x,y), 2, Scalar(0,0,255), 2, 8);
			rectangle(image,Point(left,top),Point(right,bottom),Scalar(255,0,0),1);
		}

		points0=gtPoints.clone();

		rel_pos=Mat::zeros(gtPoints.size[0],gtPoints.size[0],CV_32FC2);
		rel_weight=Mat::eye(gtPoints.size[0],gtPoints.size[0],CV_32S);

		//Good Feature to Track
		/*
		Mat goodPoints,goodWeights,goodRelPos;
		getGoodPoints(goodPoints,goodWeights,goodRelPos,gray,gtPoints,gtRegion);
		points0.push_back(goodPoints);
		rel_pos.push_back(goodRelPos);
		rel_weight.push_back(goodWeights);
		*/
		//Canny

		Mat cannyPoints,cannyWeight,cannyRelPos;
		getCannyPoints(cannyPoints,cannyWeight,cannyRelPos,gray,gtPoints,gtRegion);
		points0.push_back(cannyPoints);
		rel_pos.push_back(cannyRelPos);
		rel_weight.push_back(cannyWeight);

		rel_pos2=gtPoints.clone();
		for(int i=0;i<gt_N;i++)
		{

			Vec2f pgt = Vec2f(0,0);
			int relcount=0;
			for(int j = 0; j < rel_weight.size[0]; j++)
			{
				if(rel_weight.at<int>(j,i)>0)
				{
					pgt+=points0.at<Vec2f>(j);
					relcount++;
				}
			}
			pgt/=relcount;
			rel_pos2.at<Vec2f>(i)=gtPoints.at<Vec2f>(i)-pgt;
		}
		for(int rel_i=0;rel_i<points0.size[0];rel_i++)
		{
			// zx comment
            //Point2i p = points0.at<Vec2f>(rel_i);
			//if(p.x>=0&&p.x<image.size[1]&&p.y>=0&&p.y<image.size[0])
		//		image.at<Vec3b>(p)=Vec3b(0,255,0);
		}
		imshow("FeaturePoints",image);
		waitKey(0);
		gt_inited=true;
	}
	return 1;
}

int MyTrackerKLT::updateAframe(unsigned char* framedata)
{
		Mat frame(frame_height, frame_width, CV_8UC3, framedata);
        frame.copyTo(image);
		frame.copyTo(image2);
        cvtColor(image, gray, COLOR_BGR2GRAY);
		blur( gray, gray, Size(3,3));
		if(prevGray.empty())gray.copyTo(prevGray);
        if( gt_inited )
        {
			//Mat tmppoints=points0;
            calcOpticalFlowPyrLK(prevGray, gray, points0, points1, status, err, winSize, 3, termcrit, 0, 0.001);
			//calcOpticalFlowPyrLK(prevGray, gray, points1, tmppoints, status, err, winSize, 3, termcrit, 0, 0.001);
			int valid_count=0;
            for(int i = 0; i < points1.size[0]; i++ )
            {
				if(status[i]>0)
				{
                    //zx comment
					//Point2i p1=points1.at<Vec2f>(i);
					//if( norm(Point2i(points1.row(i)) - Point2i(points1.row(i))) <= 5 )
					//circle( image, Point2i(points1.at<Vec2f>(i)), 1, Scalar(0,255,0), -1, 8);
					//if(p1.x>=0&&p1.x<image.size[1]&&p1.y>=0&&p1.y<image.size[0])
					//	image.at<Vec3b>(p1)=Vec3b(0,255,0);
					//rel_weight.row(i)=0;
					points1.row(valid_count)=points1.row(i);
					//points1.at<Vec2f>(k)=Vec2f(p1.x,p1.y);
					rel_weight.row(valid_count)=rel_weight.row(i);
					rel_pos.row(valid_count)=rel_pos.row(i);
					valid_count++;
				}
            }
			points1.resize(valid_count);
			rel_weight.resize(valid_count);
			rel_pos.resize(valid_count);
			// zx comment
            //sprintf_s(errstrbuff,"point %d",points1.size[0]);
			std::cout<<errstrbuff<<std::endl;
			//imshow("weight",rel_weight.t()*256*256);
			//sprintf(errstrbuff,"%d",countNonZero(status));
			//cv_debug(errstrbuff);
			int trackingcount =0;
			for(int i=0;i<gt_N;i++)
			{

				Vec2f pgt = Vec2f(0,0);
				int relcount=0;
				float top=image.size[0]-1,left=image.size[1]-1,right=0,bot=0;
				for(int j = 0; j < rel_weight.size[0]; j++)
				{
					if(rel_weight.at<int>(j,i)>0)
					{
						float x=points1.at<Vec2f>(j)[0],y=points1.at<Vec2f>(j)[1];
						//pgt+=(rel_pos.at<Vec2f>(j,i)+points1.at<Vec2f>(j));
						pgt+=points1.at<Vec2f>(j);
						relcount++;
						if(x<left)
							left=x;
						if(x>right)
							right=x;
						if(y<top)
							top=y;
						if(y>bot)
							bot=y;
					}
				}

				if (relcount >0)trackingcount++;
				// zx comment
                //Point2i p1=rel_pos2.at<Vec2f>(i)+pgt/relcount;
				//circle( image2, p1, 2, Scalar(0,0,255), 2, 8);
				rectangle(image,Point2i(left,top),Point2i(right,bot),Scalar(255,0,0),1);
			}

			/*
			sprintf(fnamestr,"all/track/%06d.png",frameidx);
			imwrite(fnamestr,image2);
			sprintf(fnamestr,"all/feat/%06d.png",frameidx);
			imwrite(fnamestr,image);
			*/
			std::cout<<"tracking point:"<<trackingcount<<std::endl;

			std::swap(points1, points0);
			cv::swap(prevGray, gray);
        }

		imshow("FeaturePoints",image);
		imshow("tracking",image2);
		waitKey(1);
		return 1;
}
