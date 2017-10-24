#include "stream_hlds.h"
#include <iostream>


using namespace cv;

stream_hlds::stream_hlds()
{
    m_jpg_compressor = tjInitCompress();
    m_jpg_buf = tjAlloc(640 * 480 * 1);
}

stream_hlds::~stream_hlds()
{
    Stop();
    tjFree(m_jpg_buf);
    tjDestroy(m_jpg_compressor);
}


int stream_hlds::Proc()
{
    return 0;
}


