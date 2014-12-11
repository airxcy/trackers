#include "itf/trackers/buffers.h"

template <typename ptr_T>
QueBuff<ptr_T>::QueBuff()
{
	headidx=0;tailidx=0;
	headptr=NULL,tailptr=NULL,cur_frame_ptr=NULL,data=NULL;
}

template <typename ptr_T>
bool QueBuff<ptr_T>::init(int fbsize,int l)
{
	headidx=0;tailidx=0;
	frame_step_size=fbsize;
	frame_byte_size=frame_step_size*sizeof(ptr_T);
	que_len=l;
	data=new ptr_T[frame_step_size*que_len];
	len=0;
	headptr=data+headidx*frame_step_size;
	tailptr=data+tailidx*frame_step_size;
	cur_frame_ptr=tailptr;
	return true;
}
template <typename ptr_T>
void QueBuff<ptr_T>::clear()
{
	headidx=0;tailidx=0;
	headptr=data+headidx*frame_step_size;
	tailptr=data+tailidx*frame_step_size;
	cur_frame_ptr=tailptr;
	len=0;
}

template <typename ptr_T>
void QueBuff<ptr_T>::increPtr()
{
	tailidx=(tailidx+1)%que_len;
	if(tailidx==headidx)
	{
		headidx=(headidx+1)%que_len;
	}
	else
	{
		len++;
	}
	cur_frame_ptr=tailptr;
	headptr=data+headidx*frame_step_size;
	tailptr=data+tailidx*frame_step_size;
}
template <typename ptr_T>
void QueBuff<ptr_T>::updateAFrame(ptr_T* new_frame_ptr)
{
	memcpy(tailptr,new_frame_ptr,frame_byte_size);
	increPtr();
}
template <typename ptr_T>
ptr_T *QueBuff<ptr_T>::getPtr(int i)
{
	ptr_T* ptr= data+(headidx+i)%que_len*frame_step_size;
	return ptr;
}
template class QueBuff<BYTE>;
template class QueBuff<int>;
template class QueBuff<float>;
template class QueBuff<double>;


FrameBuff::FrameBuff()
{
	width=0,height=0,elem_byte_size=0;
}
bool FrameBuff::init(int bsize,int w, int h,int l)
{
	width=w,height=h,elem_byte_size=bsize;
	return QueBuff::init(elem_byte_size*width*height,l);
}

/*
void TrackBuff::updateAFrame(int x,int y,int t)
{
	*tailptr=x;
	*(tailptr+1)=y;
	*(tailptr+2)=t;
	increPtr();
}
*/
