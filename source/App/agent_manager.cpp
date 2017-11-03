#include<iostream>
#include "agent_manager.h"
#include "hlds_agent.h"

using namespace std;
agent_manager::agent_manager()
{
    m_index =0;
    m_tofm = new TofManager();
    InitManager();
}


agent_manager::~agent_manager()
{
}

void agent_manager::InitManager()
{
    //LoadIniFile 
    //Open TOF Manager (Read tof.ini file)
    //Get number of TOF sensor and TOF information list
    //const TofInfo * ptofinfo = nullptr;
    //int numoftof = tofm.GetTofList(&ptofinfo);
}

void agent_manager::AddHldsAgent()
{
    m_agents_lock.lock();
    hlds_agent* new_agent = new hlds_agent();
    m_hlds_agents.push_back(new_agent);
    new_agent->SetCPS(16);
    new_agent->SetThreadCount(m_index);
    new_agent->Start(m_broker_controller);
    m_agents_lock.unlock();
}

void agent_manager::DelHldsAgent()
{
    m_agents_lock.lock();
    deque<hlds_agent*>::iterator itr = m_hlds_agents.begin();
    while (itr != m_hlds_agents.end())
    {
        if ((*itr)->IsStopped() == true)
        {
            delete *itr;
            itr = m_hlds_agents.erase(itr);
            cout<<"client erase ---"<<endl;
        }
        else
        {
            ++itr;
        }
    }
    m_agents_lock.unlock();
}

void agent_manager::SaveIniFile()
{
}


int agent_manager::Proc()
{
    // to do read db_thread count //
    // if(bg_count != cur_count ) 
    // if(> ADD AddHldsAgent
    // else < DEL DelHldsAgent
    
    if(m_index < 2)
    {
        AddHldsAgent();
    }
    m_index ++;
    return 0;
}

