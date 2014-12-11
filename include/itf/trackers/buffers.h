#ifndef BUFFERS_H
#define BUFFERS_H
#include <iostream>
#include <string.h>
typedef unsigned char BYTE;
template <typename ELEM_T> class QueBuff
{
public:
	int headidx,tailidx,frame_step_size,que_len,frame_byte_size,len;
	ELEM_T *headptr, *tailptr, *cur_frame_ptr;
	ELEM_T *data;
	QueBuff();
	~QueBuff()
	{
		delete data;
	};
	bool init(int fbsize,int l);
	void clear();
	void increPtr();
	void updateAFrame(ELEM_T* new_frame_ptr);
	ELEM_T *getPtr(int i);
};



class FrameBuff : public QueBuff<BYTE>
{
public:
	int elem_byte_size,width,height;
	FrameBuff();
	bool init(int bsize,int w, int h,int l);
};

class TrackBuff :  public QueBuff<int>
{
public:
	//TrackBuff();
	//bool init(int l);
	//void updateAFrame(int x,int y,int t);
};

#endif