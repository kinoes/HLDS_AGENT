#include<iostream>
#include "event_manager.h"
#include <json/json.h>

#define WEBSOCKET_PORT 8810
using namespace std;


event_manager::event_manager()
{
	m_run_thread = thread(&event_manager::run_thread, this);
	m_uri = "http://172.16.120.223:8810/ISAVER.ADMIN"; // for test to do read database 
	m_tmsclient.init_asio();
	m_tmsclient.clear_access_channels(websocketpp::log::alevel::all);
	m_tmsclient.set_open_handler(bind(&event_manager::on_open,this,::_1));
	m_tmsclient.set_close_handler(bind(&event_manager::on_close,this,::_1));
	m_tmsclient.set_message_handler(bind(&event_manager::on_message,this,::_1,::_2));
	m_tmsclient.set_fail_handler(bind(&event_manager::on_fail,this,::_1));
	websocketpp::lib::error_code ec;
	client::connection_ptr con = m_tmsclient.get_connection(m_uri, ec);
	m_tmsclient.connect(con);

}

event_manager::~event_manager()
{
	m_websocket_thread_run_flag = false;
	Stop();
	m_tmsclient.stop();
	m_run_thread.join();
}

void event_manager::push_packet(xpacket *packet)
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

xpacket* event_manager::pop_packet()
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

void event_manager::run_thread()
{
	while (m_websocket_thread_run_flag) 
	{
		try
		{
			m_tmsclient.run();
		}
		catch(exception e)
		{
			m_tmsclient.stop();
			continue;
		}
		usleep(100);
	}
}

void event_manager::on_message(connection_hdl hdl, client::message_ptr msg)
{
	Json::Reader reader;
	Json::Value root;
	bool parsingRet = reader.parse(msg->get_payload(), root);
	if (!parsingRet) {
		return;
	}
}

void event_manager::on_open(connection_hdl hdl)
{
	cout<<"connect web"<<endl;
	web_connections_mutex.lock();
	web_connections.insert(hdl);
	web_connections_mutex.unlock();
}


void event_manager::on_close(connection_hdl hdl)
{
	cout<<"disconnect web"<<endl;
	web_connections_mutex.lock();
	web_connections.erase(hdl);
	web_connections_mutex.unlock();
}

void event_manager::on_fail(connection_hdl hdl)
{
}

int event_manager::Proc()
{
	xpacket *packet = pop_packet();
	if (packet == nullptr) {
		this_thread::sleep_for(chrono::milliseconds(10));
		return 1;
	}
	packet->ResetPos();
	Json::Value json;

	Json::StyledWriter writer;
	string json_string = writer.write(json);

	web_connections_mutex.lock();
	for (auto iter : web_connections) {
		try {
			m_tmsclient.send(iter, json_string, websocketpp::frame::opcode::text);
		} catch (websocketpp::exception e) {
		}
	}
	web_connections_mutex.unlock();
	return 0;
}

