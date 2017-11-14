#include<iostream>
#include "agent_manager.h"
#include "agent_broker.h"
#include "hlds_agent.h"

using namespace std;
agent_manager::agent_manager()
{
    m_manage_status = 2;
    m_device_info_status = false;
	InitManager();
}


agent_manager::~agent_manager()
{
}

void agent_manager::InitManager()
{
	// to do database device_count 
	// for count //
	// hlds_agent* new_agent = new hlds_agent();
	//((agent_broker*)m_arg)->AddAgent(new_agent);
}

int agent_manager::Proc()
{
	m_manage_status= ((agent_broker*)m_arg)->GetDeviceManageFlag();
	if(m_manage_status == DEVICE_ADD_CMD) 
	{
		m_device_info_status = ((agent_broker*)m_arg)->GetDeviceInfo(); // 중복처리  
		if(m_device_info_status == true)
		{
			hlds_agent* new_agent = new hlds_agent();
			((agent_broker*)m_arg)->AddAgent(new_agent);
		}
		else
		{
			m_device_info_status = 2; //OFF 
		}
	}
	else if(m_manage_status == DEVICE_DEL_CMD)
	{
		((agent_broker*)m_arg)->DelAgent();
	}
	return 0;
}

