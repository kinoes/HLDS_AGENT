#include<iostream>
#include "agent_manager.h"
#include "hlds_agent.h"

using namespace std;
agent_manager::agent_manager()
{
    m_index =0;
}


agent_manager::~agent_manager()
{
}


int agent_manager::Proc()
{
    if(m_index ==0 )
    {
        agent.SetCPS(10);
        agent.SetThreadCount(m_index); 
        agent.Start(nullptr);
    }
    else if(m_index ==1)
    {
        agent1.SetCPS(20);
        agent1.SetThreadCount(m_index);
        agent1.Start(nullptr);
    }
    m_index++;
    return 0;
}

