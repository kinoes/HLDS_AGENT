#include "stream_hlds.h"
#include "agent_broker.h"
#include <iostream>

using namespace cv;
stream_hlds::stream_hlds()
{
    m_jpg_compressor = tjInitCompress();
    m_jpg_buf = tjAlloc(640 * 480);
}

stream_hlds::~stream_hlds()
{
    Stop();
    tjFree(m_jpg_buf);
    tjDestroy(m_jpg_compressor);
}


int stream_hlds::Proc()
{
	Mat cur_img;
	if (((agent_broker*)m_arg)->GetCurImage(&cur_img) >= 0)
	{
		unsigned long compressed_len;
		//int comp_ret = tjCompress2(m_jpg_compressor, cur_img.data, cur_img.cols, 0, cur_img.rows, cur_img.channels() == 1 ? TJPF_GRAY : TJPF_BGR, &m_jpg_buf, &compressed_len,TJSAMP_444, 75, TJFLAG_NOREALLOC);

		/*if(comp_ret>0)
		{
			cout<<"STREAM=="<<endl;
			((agent_broker*)m_arg)->StreamHLDSClients(m_jpg_buf,compressed_len);
		}*/
		 Mat resize_img; 
		 resize(cur_img,resize_img, Size(640,480), 0, 0, INTER_NEAREST);
		((agent_broker*)m_arg)->StreamHLDSClients(resize_img.data,resize_img.cols*resize_img.rows*3);
	}
	return 0;
}


