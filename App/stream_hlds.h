#pragma once

#include "xthread.h"
#include <opencv2/opencv.hpp>
#include <turbojpeg.h>
using namespace std;

class stream_hlds : public xthread
{
public:
    stream_hlds();
    virtual ~stream_hlds();
private:
    virtual int Proc();
    tjhandle m_jpg_compressor;

    int m_camera_type = 0;
    uint8_t* m_jpg_buf;
};


