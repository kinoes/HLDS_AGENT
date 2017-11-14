#pragma once 

#include <deque>
#include <unordered_map>
#include <mutex>
#include "xthread.h"
#include "tof.h"
#include "hlds_agent.h"
#include "agent_broker.h"

class agent_manager : public xthread
{
    public:
        agent_manager();
        virtual ~agent_manager();
		void SetBrokerController(agent_broker* broker_controller) { m_broker_controller = broker_controller;} 
    private:
        
		uint8_t m_manage_status;        
        bool m_device_info_status;
		TofManager *m_tofm = nullptr;
		agent_broker* m_broker_controller = nullptr;
		virtual int Proc();
		DEVICEINFO m_device_info; 
		//comm
        mutex m_agents_lock;
        deque<hlds_agent*> m_hlds_agents;
        void InitManager();
};
