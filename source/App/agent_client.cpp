#include "agent_client.h"
#include "agent_broker.h"
#include "command.h"
#include <iostream>
#include <inttypes.h>

using namespace std;
agent_client::agent_client(tcp::socket* _socket)
{
    m_socket_lock.lock();
    m_socket = _socket;
    m_socket_lock.unlock();
    m_send_thread = thread(&agent_client::SendProc, this);
    m_compacket_thread = thread(&agent_client::ComPacketProc, this);
 //   m_alivecheck_mode = gmarket.alivecheck_mode;
}

agent_client::~agent_client()
{
    ClearSendQ();
    m_send_thread_stop_flag = true;
    m_send_thread.join();

    m_compacket_thread_stop_flag = true;
    m_compacket_thread.join();

    Stop();
    m_socket_lock.lock();
    SAFE_DELETE(m_socket);
    m_socket_lock.unlock();
    SAFE_DELETE(m_making_packet);
}

void agent_client::PushSendPacket(xpacket* packet)
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


void agent_client::ClearRecvQ()
{
    m_recv_q_lock.lock();
    while (m_recv_q.empty() == false)
    {
        delete m_recv_q.front();
        m_recv_q.pop_front();
    }
    m_recv_q_lock.unlock();
}

xpacket* agent_client::PopRecvPacket()
{
    xpacket* ret_packet = nullptr;
    m_recv_q_lock.lock();
    if (m_recv_q.empty() == false)
    {
        ret_packet = m_recv_q.front();
        m_recv_q.pop_front();
    }
    m_recv_q_lock.unlock();
    return ret_packet;
}

void agent_client::PushRecvPacket(xpacket* packet)
{
    m_recv_q_lock.lock();
    while (m_recv_q.size() > MAX_RECVQ_SIZE)
    {
        delete m_recv_q.front();
        m_recv_q.pop_front();
    }
    packet->ResetPos();
    m_recv_q.push_back(packet);
    m_recv_q_lock.unlock();
}


void agent_client::ClearSendQ()
{
    m_send_q_lock.lock();
    while (m_send_q.empty() == false)
    {
        delete m_send_q.front();
        m_send_q.pop_front();
    }
    m_send_q_lock.unlock();
}

xpacket* agent_client::PopSendPacket()
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


void agent_client::ComPacketProc()
{
    while(m_compacket_thread_stop_flag == false)
    {
		xpacket* recv_packet = PopRecvPacket();
		
		if (recv_packet == nullptr)
		{
			this_thread::sleep_for(chrono::milliseconds(10));
			continue;
		}
		
		uint8_t command_code = recv_packet->m_header.h_command;
			if(command_code == OP_HLDS_STREAM)
			{
				 ((agent_broker*)m_arg)->SetStreamStatus(recv_packet);
			}
			else if(command_code == OP_HLDS_DEVICE_INFO)
			{
				((agent_broker*)m_arg)->SetDeviceInfo(recv_packet);
			}
			else if(command_code == OP_STREAM_FRAME)
			{
				 ((agent_broker*)m_arg)->SetFrame(recv_packet);
			}
			else if(command_code == OP_CONFIG_SET)
			{
				  ((agent_broker*)m_arg)->SetConfig(recv_packet);
			}
			else if(command_code == OP_LENS_MOVE)
			{
				  ((agent_broker*)m_arg)->SetPantiltData(recv_packet);
			}
			else if(command_code == OP_FOCUS_MOVE)
			{
				  ((agent_broker*)m_arg)->SetFocusInOut(recv_packet);
			}
			else
			{
				cout<<"NOT COMMAND \n"<<endl;
			}
	}
}

void agent_client::SendProc()
{
	while (m_send_thread_stop_flag == false)
	{
		xpacket* send_packet = PopSendPacket();
		if (send_packet == nullptr)
		{
			this_thread::sleep_for(chrono::milliseconds(10));
			continue;
		}

		if(stream_onoff == STREAM_OFF)
		{
			printf("STREAM_OFF\n");
			if(send_packet->m_header.h_command == OP_HLDS_STREAM)
			{
				continue;
			}
		}
		bool _error = false;
		m_socket_lock.lock();
		if (m_socket != nullptr)
		{
			try
			{
				boost::asio::write(*m_socket, boost::asio::buffer(send_packet->GetBuf(), send_packet->GetTotalPackSize()));
			}
			catch (const std::exception&)
			{
				_error = true;
			}
		}
		else
			_error = true;

		m_socket_lock.unlock();
		delete send_packet;

		if (_error == true)
			break;
	}
}

int agent_client::Proc()
{
	size_t bytesReceived;
    boost::system::error_code error;
    bool _error = false;
    try
    {
        if (m_recv_step == 0)//find header prefix
        {
            while (m_prefix_buf.size() >= xpacket::PREFIX_SIZE)
            {
                m_prefix_buf.pop_front();
            }

			bytesReceived = boost::asio::read(*m_socket, boost::asio::buffer(m_recv_buf, 1));
			m_prefix_buf.push_back(m_recv_buf[0]);
            if ((int)m_prefix_buf.size() == xpacket::PREFIX_SIZE)
            {
                if (m_prefix_buf[0] == 'R' && m_prefix_buf[1] == 'X' && m_prefix_buf[2] == 'M' && m_prefix_buf[3] == 'G')//found
                {
                    m_recv_step = 1;//step to the recv whole header
                    m_msg_buf[0] = 'R'; m_msg_buf[1] = 'X'; m_msg_buf[2] = 'M'; m_msg_buf[3] = 'G';
                    m_msg_buf_pos = xpacket::PREFIX_SIZE;
                    m_prefix_buf.clear();
                    m_making_packet = new xpacket(RECV_BUF_LEN);
                }
            }
        }
        if (m_recv_step == 1)//recv whole header
        {
            int remain_header_size = xpacket::XPACKET_HEADER_SIZE - m_msg_buf_pos;
            bytesReceived = boost::asio::read(*m_socket, boost::asio::buffer(m_recv_buf, remain_header_size));

            memcpy(m_msg_buf + m_msg_buf_pos, m_recv_buf, bytesReceived);
            m_msg_buf_pos += bytesReceived;

            if (m_msg_buf_pos >= xpacket::XPACKET_HEADER_SIZE)
            {
                memcpy(&(m_making_packet->m_header), m_msg_buf, xpacket::XPACKET_HEADER_SIZE);
                m_recv_step = 2;
            }
        }
        if (m_recv_step == 2)//recv body && tail
        {
            int remain_packet_size = m_making_packet->m_header.h_packet_size - m_msg_buf_pos;
            if (remain_packet_size > RECV_BUF_LEN)
            {//error
                _error = true;
            }
            else
            {
                bytesReceived = boost::asio::read(*m_socket, boost::asio::buffer(m_recv_buf, remain_packet_size));
                memcpy(m_msg_buf + m_msg_buf_pos, m_recv_buf, bytesReceived);
                m_msg_buf_pos += bytesReceived;

                if (m_msg_buf_pos >= m_making_packet->m_header.h_packet_size)//complete
                {
                    if (m_msg_buf[m_msg_buf_pos - 2] == 0xFE && m_msg_buf[m_msg_buf_pos - 1] == 0xFF)
                    {
                        if (m_making_packet->m_header.h_packet_size > xpacket::XPACKET_HEADER_SIZE + xpacket::XPACKET_TAIL_SIZE)
                            m_making_packet->PushMem(m_msg_buf + xpacket::XPACKET_HEADER_SIZE, m_making_packet->m_header.h_packet_size - xpacket::XPACKET_HEADER_SIZE - xpacket::XPACKET_TAIL_SIZE);
                        m_making_packet->Pack();
                        PushRecvPacket(m_making_packet);
                        m_making_packet = nullptr;
                    }
                    m_msg_buf_pos = 0;
                    m_recv_step = 0;
                }
            }
        }
    }
    catch (...)//discon
    {
        _error = true;
    }

    if (_error == true)
    {
        cout<<"DELETE"<<endl;
        m_socket_lock.lock();
        SAFE_DELETE(m_socket);
        m_socket_lock.unlock();
        return DEATH_CODE;
    }
    return 0;
}
