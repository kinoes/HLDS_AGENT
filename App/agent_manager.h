#pragma once 

#include <deque>
#include <unordered_map>
#include <mutex>
#include "xthread.h"
#include "tof.h"
#include "hlds_agent.h"

class agent_manager : public xthread
{
    public:
        agent_manager();
        virtual ~agent_manager();
    private:
        int m_index;
        
        TofManager *m_tofm = nullptr;
        virtual int Proc();
        
        //comm
        mutex m_agents_lock;
        deque<hlds_agent*> m_hlds_agents;
        void AddHldsAgent();
        void DelHldsAgent();
        void SaveIniFile();
        void InitManager();
};
