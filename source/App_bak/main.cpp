#include<iostream>
#include <chrono>
#include <thread>
#include <tuple>

#include "xthread.h"
#include "agent_manager.h"
#include "event_manager.h"
#include "accept_manager.h"
#include "agent_broker.h"
#include "stream_hlds.h"
#include "xpacket.h"
#include "config.h"
#include "gmark.h"
#include "logger.h"


Config *config;
GMarket gmarket;
LogMgrC *zetavulog;
DebugLogger *dbglog;

using namespace std;


/////////////////////////////////////////////////////////////////////////
// Name : initconfig
// Function : INIT CONFIG
/////////////////////////////////////////////////////////////////////////
void initconfig()
{
}

void initlog(const char *procname)
{

}


int main(int argc, char** argv)
{
	// thread controller start // 
	agent_broker agent_broker_th; 
	agent_broker_th.SetCPS(1000);
	agent_broker_th.Start(&agent_broker_th);

	agent_manager manager_th;
    manager_th.SetCPS(10);
    manager_th.SetBrokerController(&agent_broker_th);
	manager_th.Start(&agent_broker_th);

	accept_manager accept_manager_th;
	accept_manager_th.SetCPS(10);
	accept_manager_th.Start(&agent_broker_th);

	event_manager event_manager_th;
    event_manager_th.SetCPS(30);
    event_manager_th.Start(&agent_broker_th);

	stream_hlds stream_hlds_th; 
	stream_hlds_th.SetCPS(15); 
	stream_hlds_th.Start(&agent_broker_th);

    //for thread status manage// 
	while(true)
    {
        this_thread::sleep_for(chrono::milliseconds(100));
    }
    return 0;
}
