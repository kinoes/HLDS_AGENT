#pragma once

#include "xthread.h"
#include <opencv2/opencv.hpp>
using namespace std;

class stream_hlds : public xthread
{
public:
    stream_hlds();
    virtual ~stream_hlds();
private:
    virtual int Proc();
    int m_camera_type = 0;
    uint8_t* m_jpg_buf;
};


