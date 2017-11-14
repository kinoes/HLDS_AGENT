#include<iostream>
#include <chrono>
#include <thread>
#include <tuple>

#include "xthread.h"
#include "agent_manager.h"
#include "event_manager.h"
#include "accept_manager.h"
#include "agent_websocket.h"
#include "agent_broker.h"
#include "stream_hlds.h"
#include "xpacket.h"
#include "config.h"
#include "gmark.h"
#include "logger.h"
#include "deviceinfo_db.h"
#include "event_db.h"
#include "agent_status_db.h"


Config *config;
GMarket gmarket;
LogMgrC *hldslog;
DebugLogger *dbglog;

using namespace std;


void initdatabase()
{

	string deviceinfodb = config->str("DATABASE", "deviceinfo_db_file");
	deviceinfo_db::Initialization(deviceinfodb.c_str());

	string eventdb = config->str("DATABASE", "event_db_file");
	cout<<"event:"<<eventdb<<endl;
	event_db::Initialization(eventdb.c_str());

	string agentstatusdb = config->str("DATABASE", "agent_status_db_file");
	agent_status_db::Initialization(agentstatusdb.c_str());

}

void exitprogram(int sig)
{
	sleep(1);
	_exit(0);
}
/////////////////////////////////////////////////////////////////////////
// Name : initconfig
// Function : INIT CONFIG
/////////////////////////////////////////////////////////////////////////
void initconfig()
{
	char path[128];
	sprintf(path, "%s/HD_PROJECT/config/hldsagent.conf",getenv("HOME"));
	cout<<"CONPATH:"<<path<<endl;
	config = new Config();
	config->init(path);
	
	string deviceinfodb = config->str("DATABASE", "deviceinfo_db_file");
	cout<<"deviceinfodb:"<<deviceinfodb<<endl;

	gmarket.test = config->getiValue("TOF","ID");
	cout<<"ID:"<<gmarket.test<<endl;

}

void initlog(const char *procname)
{
	char path[128];
	sprintf(path, "%s/HD_Project/log/%s/hlds_agent.log", getenv("HOME"),procname);
	//hldslog = new LogMgrC(path, "hlds_agent");
}

void init_start()
{
	cout<< "init config start"<<endl;
	initconfig();
	cout<< "--------------------------------------->"<<"\n\n";

	char procname[15];
	sprintf(procname, "HLDS_AGENT");
	initlog(procname);
	cout<< "init log start"<<endl;
	cout<< "--------------------------------------->"<<"\n\n";

	cout<< "init database start"<<endl;
	initdatabase();
	cout<< "--------------------------------------->"<<"\n\n";

	signal(SIGTERM, exitprogram);
	signal(SIGINT , exitprogram);
	signal(SIGPIPE, SIG_IGN);
	signal(SIGCHLD, SIG_IGN);
	signal(SIGHUP , SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
	signal(SIGUSR1, SIG_IGN);
	signal(SIGUSR2, SIG_IGN);
	signal(SIGTSTP, SIG_IGN);
	signal(SIGALRM, SIG_IGN);

	srandom(getpid());
}

int main(int argc, char** argv)
{
	init_start();
	// thread controller start // 
	agent_broker agent_broker_th; 
	agent_broker_th.SetCPS(1000);
	agent_broker_th.Start(&agent_broker_th);

	agent_manager agent_manager_th;
	agent_manager_th.SetCPS(10);
	agent_manager_th.Start(&agent_broker_th);

	accept_manager accept_manager_th;
	accept_manager_th.SetCPS(10);
	accept_manager_th.Start(&agent_broker_th);
	
	event_manager event_manager_th;
    event_manager_th.SetCPS(15);
    event_manager_th.Start(&agent_broker_th);

	stream_hlds stream_hlds_th; 
	stream_hlds_th.SetCPS(16); 
	stream_hlds_th.Start(&agent_broker_th);

	agent_websocket agent_websocket_th;
	agent_websocket_th.SetCPS(15);
	agent_websocket_th.Start(&agent_broker_th);
	
	cout<< "--------------------------------------->"<<"\n";
	vector<tuple<xthread*,string>> thread_set;

	//for thread status manage// 
	
	thread_set.push_back(tuple<xthread*, string>(&agent_broker_th, "broker"));
	thread_set.push_back(tuple<xthread*, string>(&agent_manager_th, "agent"));
	thread_set.push_back(tuple<xthread*, string>(&accept_manager_th, "accept"));
	thread_set.push_back(tuple<xthread*, string>(&event_manager_th, "event"));
	thread_set.push_back(tuple<xthread*, string>(&stream_hlds_th, "stream"));
	thread_set.push_back(tuple<xthread*, string>(&agent_websocket_th, "web"));
    
	const double abnormal_time_limit = 10.0;//second
	while (true)
	{
		chrono::time_point<chrono::system_clock> now = chrono::system_clock::now();
		for (int i = 0; i < (int)thread_set.size(); i++)
		{
			xthread *t_thread = get<0>(thread_set[i]);
			chrono::duration<double> el_sec = now - t_thread->m_last_ok_tc;

			if (el_sec.count() > abnormal_time_limit)
			{
				string thread_name = get<1>(thread_set[i]);
			}
		}
		this_thread::sleep_for(chrono::milliseconds(100));
	}

	return 0;
}
