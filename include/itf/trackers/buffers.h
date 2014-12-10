#ifndef BUFFERS_H
#define BUFFERS_H
#include <iostream>
#include <string.h>
typedef unsigned char BYTE;
template <typename ptr_T> class QueBuff
{
public:
	int headidx,tailidx,frame_step_size,que_len,frame_byte_size,len;
	ptr_T *headptr, *tailptr, *cur_frame_ptr;
	ptr_T *data;
	QueBuff();
	~QueBuff()
	{
		delete data;
	};
	bool init(int fbsize,int l);
	void clear();
	void increPtr();
	void updateAFrame(ptr_T* new_frame_ptr);
	ptr_T *getPtr(int i);
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
	TrackBuff();
	bool init(int l);
	//void updateAFrame(int x,int y,int t);
};
#endif