#include<iostream>
#include "agent_broker.h"


using namespace std;
agent_broker::agent_broker()
{
}


agent_broker::~agent_broker()
{
	Stop();
	if(m_cur_hlds_img == nullptr)
	{
		delete [] m_cur_hlds_img;
	}
}


void agent_broker::SetStreamStatus(xpacket* packet)
{       
	uint8_t mode;
	if (packet->PopByte(mode) < 0)
	{   
		return;
	}   
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

void agent_broker::SetPantiltData(xpacket* packet)
{       
	uint8_t mode;
	if (packet->PopByte(mode) < 0)
	{   
		return;
	}   
}       

void agent_broker::SetFocusInOut(xpacket* packet)
{       
	uint8_t mode;
	if (packet->PopByte(mode) < 0)
	{   
		return;
	}   
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

	m_clients_lock.lock();
	for (int i = 0; i < (int)m_clients.size(); i++)
	{
		if (m_clients[i]->IsStopped() == true)
			continue;

		for (int k = 0; k < (int)slices.size(); k++)
			m_clients[i]->PushSendPacket(slices[k]);
	}
	m_clients_lock.unlock();
	
	for (int k = 0; k < (int)slices.size(); k++)
	{
		delete slices[k];
	}
}


int agent_broker::Proc()
{
    return 0;
}

