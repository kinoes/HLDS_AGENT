#pragma once 
#include <opencv2/opencv.hpp>
#include "xthread.h"
#include "tof.h"

using namespace std;
using namespace hlds;


class hlds_agent : public xthread
{
    public:
        hlds_agent();
        virtual ~hlds_agent();
        void SetThreadCount(int index);
        Tof* GetTofInstance() { return m_Tof; }
        TofManager* GetTofManagerInstance(){return m_TofManager;}
        TofInfo* GetTofInfo(){return m_TofInfo;}

    private:
        int m_index;
        Tof *m_Tof = nullptr;
        TofManager *m_TofManager = nullptr;
        TofInfo *m_TofInfo = nullptr;
        virtual int Proc();
};
