#include<iostream>
#include "agent_broker.h"


using namespace std;
agent_broker::agent_broker()
{
	m_stream_onoff = 0; //default stream OFF
	m_zoom = 0.0f;
	m_zoom_flag = false;
	m_cam_angle_flag = false;
	m_cam_move_flag = false;
	m_etc_config_flag = false;
	m_device_manage_flag = 2;
}


agent_broker::~agent_broker()
{
	Stop();
	if(m_cur_hlds_img == nullptr)
	{
		delete [] m_cur_hlds_img;
	}
}

void agent_broker::InitFalg()
{
	m_zoom_flag = false;
	m_cam_angle_flag = false;
	m_cam_move_flag = false;
	m_etc_config_flag = false;
	m_device_manage_flag = 2;
}

void agent_broker::AddAgent(hlds_agent* agent)
{
	m_agentcheck_manage_lock.lock();
	m_agents.push_back(agent);
	agent->SetCPS(16);
	agent->SetDeviceInfo(m_device_info);
	agent->Start(this);
	m_device_manage_flag = 2; // OFF
	m_agentcheck_manage_lock.unlock();
}

void agent_broker::DelAgent()
{	
	m_agentcheck_manage_lock.lock();
	deque<hlds_agent*>::iterator itr = m_agents.begin();
	while (itr != m_agents.end())
	{
		if ((*itr)->IsStopped() == true)
		{
			delete *itr;
			itr = m_agents.erase(itr);
			cout<<"agent erase ---"<<endl;
		}
		else
		{
			++itr;
		}
		if((*itr)->CompareDeviceInfo(m_device_info) == true)
		{
			delete *itr;
			itr = m_agents.erase(itr);
			cout<<"agent erase ---"<<endl;
		}
	}
	// to do delete database device_info 
	m_agentcheck_manage_lock.unlock();
}

void agent_broker::AddClient(agent_client* client)
{
	m_agent_clients_lock.lock();
	m_clients.push_back(client);
	client->Start(this);
	m_agent_clients_lock.unlock();
}


void agent_broker::SetStreamStatus(xpacket* packet)
{       
	m_stream_mode_lock.lock();
	uint8_t sub_command;

	if (packet->PopByte(sub_command) < 0)
	{
		return;
	}
	if(sub_command == SET_STREAM_ONOFF)
	{
		packet->PopByte(m_stream_onoff);
	}
	m_stream_mode_lock.unlock();
}       

uint8_t agent_broker::GetStreamStatus()
{
	return m_stream_onoff;
}


void agent_broker::SetDeviceCheck(xpacket* packet)
{
	// to do get DEVICEINFO and compare //
	m_device_check_lock.lock();
	deque<hlds_agent*>::iterator itr = m_agents.begin();
	while (itr != m_agents.end())
	{
		if ((*itr)->IsStopped() == true)
		{
			delete *itr;
			itr = m_agents.erase(itr);
			cout<<"agent erase ---"<<endl;
		}
		else
		{
			++itr;
		}
		if( (*itr)->CompareDeviceInfo(m_device_info) == true)
		{
			(*itr)->SetRunFlag(true);
		}
		else
		{
			(*itr)->SetRunFlag(false);
		}
	}
	m_device_check_lock.unlock();
}

void agent_broker::SetDeviceInfo(xpacket* packet)
{       
	uint8_t mode;
	if (packet->PopByte(mode) < 0)
	{   
		return;
	}   
}       

void agent_broker::InitStatusRes()
{
}

void agent_broker::SetFrame(xpacket* packet)
{       
	uint8_t mode;
	if (packet->PopByte(mode) < 0)
	{   
		return;
	}   
}       

void agent_broker::SetConfig(xpacket* packet)
{       
	uint8_t mode;
	if (packet->PopByte(mode) < 0)
	{   
		return;
	}   
}       

void agent_broker::ManageDeviceAddDel(xpacket* recv)
{
	m_device_manage_lock.lock();
	uint8_t sub_command;
	if (recv->PopByte(sub_command) < 0)
	{
		return;
	}
	if(sub_command == DEVICE_ADD_CMD)
	{
		m_device_manage_flag= DEVICE_ADD_CMD;
	}
	else if( sub_command == DEVICE_DEL_CMD)
	{
		 m_device_manage_flag= DEVICE_DEL_CMD;	
	}
	int lengths = sizeof(DEVICEINFO);
	uint8_t *data = new uint8_t[lengths];
	recv->PopMem((uint8_t*)data,lengths);
	memcpy(&m_device_info, data,lengths);
	delete data;
	m_device_manage_lock.unlock();
}


bool agent_broker::GetDeviceInfo()
{
	m_device_exits_lock.lock();
	deque<hlds_agent*>::iterator itr = m_agents.begin();
	if(m_agents.empty() == true)
	{
		m_device_exits_flag = true;
	}
	else
	{
		while (itr != m_agents.end())
		{
			if ((*itr)->IsStopped() == true)
			{
				delete *itr;
				itr = m_agents.erase(itr);
				cout<<"agent erase ---"<<endl;
			}
			else
			{
				++itr;
			}
			/*if( (*itr)->CompareDeviceInfo(m_device_info) == true)
			  {
			  cout<<"COMPARE TRUE"<<endl;
			  m_device_exits_flag = false;
			  }
			  else
			  {
			  cout<<"COMPARE FALSE"<<endl;
			  m_device_exits_flag = true; 
			  }*/
			m_device_exits_flag = true;
		}
	}
	m_device_exits_lock.unlock();	
	return m_device_exits_flag;
}

void agent_broker::SetEtcConfigSet(xpacket* recv)
{
	m_etc_config_data_lock.lock();
	uint8_t sub_command;
	if (recv->PopByte(sub_command) < 0)
	{
		return;
	}
    m_etc_config_flag= true;
    int lengths = sizeof(ETCCONFIG);
    uint8_t *data = new uint8_t[lengths];
    recv->PopMem((uint8_t*)data,lengths);
	memcpy(&m_etc_config_data, data,lengths);
    delete data;
	m_etc_config_data_lock.unlock();
}

ETCCONFIG agent_broker::GetEtcConfig()
{
    m_cam_angle_flag = false;
    return m_etc_config_data;
}


void agent_broker::SetPantiltData(xpacket* packet)
{       
	m_camera_move_lock.lock();
	m_cam_move_flag = true;
	uint8_t sub_command;
	if (packet->PopByte(sub_command) < 0)
	{   
		return;
	}   

	if(sub_command == SET_UP)
	{
		m_cam_status = 1; //UP 
	}
	else if(sub_command == SET_DOWN) 
	{
		m_cam_status = 2; //DOWN
	}
	else if( sub_command == SET_LEFT) 
	{
		m_cam_status = 3; //LEFT
	}
	else if(sub_command == SET_RIGHT)   
	{
		m_cam_status = 4; //RIGHT
	}
	m_camera_move_lock.unlock();

}       

uint8_t agent_broker::GetCamMoveStatus()
{
	m_cam_move_flag = false;
	return m_cam_status;
}

void agent_broker::SetFocusInOut(xpacket* packet)
{       
	m_focus_mode_lock.lock();
	m_zoom_flag = true;	
	uint8_t sub_command; 
	if (packet->PopByte(sub_command) < 0)
	{   
		return;
	}   
	if(sub_command == SET_ZOOMP) 
	{
		m_zoom = - 0.001;
	}
	else if(sub_command == SET_ZOOMN)
	{
		m_zoom =  0.001;
	}
	else
	{
		cout<<"NOT COMMAND"<<endl;
	}
	m_focus_mode_lock.unlock();
}   

float agent_broker::GetFocusInOut()
{
	m_zoom_flag = false;
	return m_zoom;
}

void agent_broker::GetAgentStatus(xpacket *packet)
{
	// to do 
}

CAMERAANGLE agent_broker::GetCameraAngle()
{
	m_cam_angle_flag = false;
	return m_camera_angle_data;
}

void agent_broker::SetCameraAngle(xpacket* recv)
{
	m_camera_angle_lock.lock();
	m_cam_angle_flag = true;
	int lengths = sizeof(CAMERAANGLE);
	
	uint8_t sub_command; 

	if (recv->PopByte(sub_command) < 0)
	{
		return;
	}

	uint8_t *data = new uint8_t[lengths];
	recv->PopMem((uint8_t*)data,lengths);
	memcpy(&m_camera_angle_data, data,16);
	delete data;
	m_camera_angle_lock.unlock();
}

void agent_broker::UpdateHLDSImage(Mat* img, bool use_lock)
{
	if(use_lock == true)
		m_img_set_lock.lock();
	if (m_cur_hlds_img!= nullptr)
		delete m_cur_hlds_img;
	m_cur_hlds_img = img;
	if (use_lock == true)
		m_img_set_lock.unlock();
}


int agent_broker::GetCurImage(Mat* out_img)
{
	m_img_set_lock.lock();
	if (m_cur_hlds_img== nullptr)
	{
		m_img_set_lock.unlock();
		return -1;
	}
	*out_img = m_cur_hlds_img->clone();
	m_img_set_lock.unlock();
	return 0;
}


void agent_broker::StreamHLDSClients(uint8_t* jpg_data, uint32_t jpg_len)
{
	xpacket stream_packet(jpg_len + DEFAULT_MAX_PACKET_UNIT_SIZE);
	stream_packet.m_header.h_command = OP_HLDS_STREAM;

	stream_packet.PushDWord(jpg_len);
	stream_packet.PushMem(jpg_data, jpg_len);
	stream_packet.Pack();
//	m_websocket_controller->push_packet(&hlds_stream_packet); for add webpage 
	vector<xpacket*> slices;
	xpacket::Split(&stream_packet, slices, DEFAULT_MAX_PACKET_UNIT_SIZE);

	m_agent_clients_lock.lock();
	for (int i = 0; i < (int)m_clients.size(); i++)
	{
		if (m_clients[i]->IsStopped() == true)
			continue;

		for (int k = 0; k < (int)slices.size(); k++)
			m_clients[i]->PushSendPacket(slices[k]);
	}
	m_agent_clients_lock.unlock();
	
	for (int k = 0; k < (int)slices.size(); k++)
	{
		delete slices[k];
	}
}


int agent_broker::Proc()
{
    return 0;
}

