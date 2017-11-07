#include "agent_websocket.h"
#include <json/json.h>
#include "agent_broker.h"
#include "command.h"

#define WEBSOCKET_PORT 8890
agent_websocket::agent_websocket()
{
    m_run_thread = thread(&agent_websocket::run_thread, this);
}

agent_websocket::~agent_websocket()
{
    m_websocket_thread_run_flag = false;
    Stop();
    m_webserver.stop();
    m_run_thread.join();
}


void agent_websocket::push_packet(xpacket *packet)
{
	xpacket* new_packet = new xpacket(packet);

	m_send_q_lock.lock();
	while (m_send_q.size() > MAX_SENDQ_SIZE)
	{
		delete m_send_q.front();
		m_send_q.pop_front();
	}
	m_send_q.push_back(new_packet);
	m_send_q_lock.unlock();
}

xpacket* agent_websocket::pop_packet()
{
	xpacket* ret_packet = nullptr;
	m_send_q_lock.lock();
	if (m_send_q.empty() == false)
	{
		ret_packet = m_send_q.front();
		m_send_q.pop_front();
	}
	m_send_q_lock.unlock();
	return ret_packet;
}

int agent_websocket::Proc()
{
	xpacket *packet = pop_packet();

	if (packet == nullptr) {
		this_thread::sleep_for(chrono::milliseconds(10));
		return 1;
	}
	packet->ResetPos();
	Json::Value json;
	if(packet->m_header.h_command == OP_HLDS_STREAM)
	{
		uint32_t jpg_size;
		packet->PopDWord(jpg_size);

		uint8_t* jpg_data = new uint8_t[jpg_size];
		packet->PopMem(jpg_data, jpg_size);

		string encoded_string = websocketpp::base64_encode(jpg_data, jpg_size);
		delete [] jpg_data;
		json["hlds"] = encoded_string;	
	}
	else if(packet->m_header.h_command == OP_HLDS_DEVICE_INFO)
	{
	}
	else if(packet->m_header.h_command == OP_HLDS_STREAM)
	{
	}

	else if(packet->m_header.h_command == OP_CONFIG_SET)
	{
	}

	else if(packet->m_header.h_command == OP_FOCUS_MOVE)
	{
	}
	else if(packet->m_header.h_command == OP_LENS_MOVE)
	{
	}
	else
	{
	}
	return 0;
}

void agent_websocket::run_thread()
{
	while (m_websocket_thread_run_flag) {

		m_webserver.init_asio();

		m_webserver.clear_access_channels(websocketpp::log::alevel::all);
		m_webserver.set_open_handler(bind(&agent_websocket::on_open,this,::_1));
		m_webserver.set_close_handler(bind(&agent_websocket::on_close,this,::_1));
		m_webserver.set_message_handler(bind(&agent_websocket::on_message,this,::_1,::_2));
		m_webserver.set_reuse_addr(true);

		boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::tcp::v4(), WEBSOCKET_PORT);
		m_webserver.listen(endpoint);
		m_webserver.start_accept();

		try
		{
			m_webserver.run();
		}
		catch(exception e)
		{
			m_webserver.stop();
			continue;
		}
	}
}

void agent_websocket::on_open(connection_hdl hdl)
{
	cout<<"connect web"<<endl;
	web_connections_mutex.lock();
	overconnect_status = false;
	web_connections.insert(hdl);
	xpacket recv_packet;
	recv_packet.m_header.h_command = OP_INIT_STATUS_REQ;
	recv_packet.Pack();
	((agent_broker*)m_arg)->InitStatusRes();
	web_connections_mutex.unlock();	
}

void agent_websocket::on_close(connection_hdl hdl)
{
	cout<<"disconnect web"<<endl;
	web_connections_mutex.lock();
	web_connections.erase(hdl);
	web_connections_mutex.unlock();
}

void agent_websocket::on_message(connection_hdl hdl, web_server::message_ptr msg)
{
	Json::Reader reader;
	Json::Value root;
	bool parsingRet = reader.parse(msg->get_payload(), root);
	if (!parsingRet) {
		return;
	}

	xpacket *recv_packet = new xpacket();
	uint8_t sub_command; 

	if (root["command"] == "OP_HLDS_STREAM")
	{
		recv_packet->m_header.h_command = OP_HLDS_STREAM;
		sub_command = (uint8_t)root["sub_command"].asInt();
		recv_packet->PushByte(sub_command);
		switch(sub_command) 
		{
			case SET_STREAM_STATUS: 
				break;
			case GET_STREAM_STATUS: 
				break;
			default:
				break;
		}
	}
	else if (root["command"] == "OP_HLDS_DEVICE_INFO")
	{
		recv_packet->m_header.h_command = OP_HLDS_DEVICE_INFO;
		sub_command = (uint8_t)root["sub_command"].asInt();
		recv_packet->PushByte(sub_command);
		switch(sub_command)
		{
			case SET_STREAM_STATUS:
				break;
			case GET_STREAM_STATUS:
				break;
			default :break;
		}
	}
	else if (root["command"] == "OP_STREAM_FRAME")
	{
		recv_packet->m_header.h_command = OP_HLDS_STREAM;
		sub_command = (uint8_t)root["sub_command"].asInt();
		recv_packet->PushByte(sub_command);
		switch(sub_command)
		{
			case SET_FRAME_VALUE:
				break;
			case GET_FRAME_VALE:
				break;
			default : 
				break;
		}
	}
	else if (root["command"] == "OP_CONFIG_SET")
	{
		recv_packet->m_header.h_command = OP_CONFIG_SET;
		sub_command = (uint8_t)root["sub_command"].asInt();
		recv_packet->PushByte(sub_command);
		switch(sub_command)
		{
			case SET_IP_INFO :
				break;
			case	GET_IP_INFO :
				break;
			case SET_MAC_INFO:
				break;
			case GET_MAC_INFO:
				break;
			case SET_DEVICE_ID:
				break;
			case GET_DEVICE_ID:
				break;
			case SET_AREA_INFO:
				break;
			case GET_AREA_INFO:
				break;
			default:
				break;
		}
	}
	else if (root["command"] == "OP_LENS_MOVE")
	{
		recv_packet->m_header.h_command = OP_LENS_MOVE;
		sub_command = (uint8_t)root["sub_command"].asInt();
		recv_packet->PushByte(sub_command);
		switch(sub_command)
		{
			case SET_OFF:
				break;
			case SET_UP:
				break;
			case SET_DOWN:
				break;
			case SET_LEFT:
				break;
			case SET_RIGHT:
				break;
			default: 
				break;
		}
	}
	else if (root["command"] == "OP_LENS_FOCUS")
	{
		recv_packet->m_header.h_command = OP_LENS_MOVE;
		sub_command = (uint8_t)root["sub_command"].asInt();
		recv_packet->PushByte(sub_command);
		switch(sub_command)
		{
			case SET_ZOOMP:
				break;
			case SET_ZOOMN:
				break;
			default:
				break;
		}
	}
	else if (root["command"] == "OP_INIT_STATUS_REQ")
	{
		recv_packet->m_header.h_command = OP_INIT_STATUS_REQ;
		sub_command = (uint8_t)root["sub_command"].asInt();
		recv_packet->PushByte(sub_command);
		switch(sub_command)
		{
		}
	}
	else
	{
		printf("NOT COMMAND\n");
	}
}
