#pragma once 
#include "xthread.h"
#include "hlds_agent.h"
class agent_manager : public xthread
{
    public:
        agent_manager();
        virtual ~agent_manager();
    private:
        int m_index;
        virtual int Proc();
        hlds_agent agent;
        hlds_agent agent1;
        hlds_agent agent2;
};
