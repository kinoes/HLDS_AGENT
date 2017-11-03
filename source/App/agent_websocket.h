#ifndef     __WEBSOCKET_THREAD_H__
#define     __WEBSOCKET_THREAD_H__

#include <thread>
#include <mutex>
#include <set>

// boost header //
// #include <boost/asio.hpp>
// #include <boost/bind.hpp>

// websocket //
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
#include <websocketpp/base64/base64.hpp>

#include "xpacket.h"
#include "xthread.h"

using namespace std;

using websocketpp::connection_hdl;
using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;

typedef websocketpp::server<websocketpp::config::asio> web_server;
typedef std::set<connection_hdl,std::owner_less<connection_hdl>> web_connection;

class agent_websocket: public xthread
{
public:
    agent_websocket();
    virtual ~agent_websocket();
    const static int MAX_SENDQ_SIZE = 2048;
    void push_packet(xpacket *packet);

private:
    virtual int Proc();

    thread m_run_thread;

    bool m_websocket_thread_run_flag = true;
    bool m_lpr_stream_framedrop = false;
    bool m_context_stream_framedrop = false;
    bool overconnect_status = false;

    web_server m_webserver;
    web_connection web_connections;
    mutex web_connections_mutex;

    mutex m_send_q_lock;
    deque<xpacket*> m_send_q;

    void run_thread();

    void on_open(connection_hdl hdl);
    void on_close(connection_hdl hdl);
    void on_message(connection_hdl hdl, web_server::message_ptr msg);

    xpacket* pop_packet();
};

#endif
