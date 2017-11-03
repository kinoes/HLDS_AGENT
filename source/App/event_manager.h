#pragma once 
#include "xthread.h"
#include "event_manager.h"
class event_manager : public xthread
{
    public:
        event_manager();
        virtual ~event_manager();
    private:
        int m_index;
        virtual int Proc();
};
