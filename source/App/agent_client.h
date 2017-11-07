#pragma once

#include "xthread.h"
#include "xpacket.h"
#include "command.h"
#include <thread>
#include <utility>
#include <deque>
#include <boost/asio.hpp>

#define SAFE_DELETE(x) { if(x != nullptr) { delete x; x = nullptr;} }

using namespace std;
using boost::asio::ip::tcp;

const int STREAM_OFF = 0;
const int STREAM_ON = 1;

class agent_client : public xthread 
{
    public: 

        const static int RECV_BUF_LEN = 256;
        agent_client(tcp::socket* _socket);
        virtual ~agent_client();

        const static int MAX_RECVQ_SIZE = 32;
        const static int MAX_SENDQ_SIZE = 2048;
        void PushSendPacket(xpacket* packet);

    private: 
        virtual int Proc();

        bool m_compacket_thread_stop_flag = false;
        thread m_compacket_thread;
        void ComPacketProc();

        void ClearRecvQ();
        xpacket* PopRecvPacket();
        void PushRecvPacket(xpacket* packet);
        mutex m_recv_q_lock;
        deque<xpacket*> m_recv_q;

        bool m_send_thread_stop_flag = false;
        thread m_send_thread;

        void ClearSendQ();
        xpacket* PopSendPacket();
        void SendProc();

        mutex  m_send_q_lock;
        deque<xpacket*> m_send_q;

		uint8_t stream_onoff = 1; //default OFF
        chrono::time_point<chrono::system_clock> m_start_time;
        chrono::time_point<chrono::system_clock> m_end_time;
        chrono::duration<double> m_el_sec;
        mutex m_socket_lock;
        tcp::socket* m_socket;
        int m_recv_step = 0;
        deque<uint8_t> m_prefix_buf;
        uint8_t m_recv_buf[RECV_BUF_LEN], m_msg_buf[RECV_BUF_LEN];
        size_t  m_msg_buf_pos = 0;
        xpacket *m_making_packet = nullptr;
};

