#include<iostream>
#include "xthread.h"
#include "agent_manager.h"
#include "hlds_agent.h"
#include "event_manager.h"
#include "accept_manager.h"

int main(int argc, char** argv)
{
    agent_manager manager_th;
    manager_th.SetCPS(1);
    manager_th.Start(nullptr);
    
    event_manager event_manager_th;
    event_manager_th.SetCPS(10);
    event_manager_th.Start(nullptr);

    accept_manager accept_manager_th;
    accept_manager_th.SetCPS(10);
    accept_manager_th.Start(nullptr);

    while(true)
    {
        this_thread::sleep_for(chrono::milliseconds(100));
    }
    return 0;
}
