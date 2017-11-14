#pragma once 
#include "xthread.h"
#include "agent_broker.h"
#include "event_manager.h"
#include <thread>
#include <mutex>
#include <set>
#include <iostream>
#include <websocketpp/config/asio_no_tls_client.hpp>
#include <websocketpp/client.hpp>
#include <websocketpp/base64/base64.hpp>

#include "xpacket.h"
#include "xthread.h"

using namespace std;


using websocketpp::connection_hdl;
using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;

typedef websocketpp::client<websocketpp::config::asio_client> client;
typedef std::set<connection_hdl,std::owner_less<connection_hdl>> web_connection;
typedef websocketpp::config::asio_client::message_type::ptr message_ptr;

class event_manager : public xthread
{
	public:
		event_manager();
		virtual ~event_manager();
		void push_packet(xpacket *packet);
		const static int MAX_SENDQ_SIZE = 2048;
    private:
		void on_open(connection_hdl hdl);
		void on_close(connection_hdl hdl);
		void on_fail(connection_hdl hdl);
		void on_message(connection_hdl hdl, client::message_ptr msg);
		void run_thread();

		deque<xpacket*> m_send_q;
		thread m_run_thread;
		client m_tmsclient;
		web_connection web_connections;
		mutex web_connections_mutex;
		mutex m_send_q_lock;
		std::string m_uri;
		xpacket* pop_packet();
		bool m_websocket_thread_run_flag = true;
		virtual int Proc();
};
