#include "stream_hlds.h"
#include "agent_broker.h"
#include <iostream>
using namespace cv;
stream_hlds::stream_hlds()
{
}

stream_hlds::~stream_hlds()
{
    Stop();
}

int stream_hlds::Proc()
{
	Mat cur_img;
	if(((agent_broker*)m_arg)->GetStreamStatus() > 0) 
	{
		if (((agent_broker*)m_arg)->GetCurImage(&cur_img) >= 0)
		{
			unsigned long compressed_len;
			Mat resize_img; 
			resize(cur_img,resize_img, Size(640,480), 0, 0, INTER_NEAREST);
			((agent_broker*)m_arg)->StreamHLDSClients(resize_img.data,resize_img.cols*resize_img.rows*3);
			 //((agent_broker*)m_arg)->StreamHLDSClients(cur_img.data,cur_img.cols*cur_img.rows*3);
		}
	}
	return 0;
}


