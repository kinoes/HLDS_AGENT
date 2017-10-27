#pragma once 
#include "xthread.h"
#include "agent_broker.h"
class agent_broker : public xthread
{
    public:
        agent_broker();
        virtual ~agent_broker();
    private:
        int m_index;
        virtual int Proc();
};
