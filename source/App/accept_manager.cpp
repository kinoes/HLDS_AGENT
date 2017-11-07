#include "accept_manager.h"
#include "agent_client.h"
#include "agent_broker.h"
#include <iostream>

using namespace std;
accept_manager::accept_manager()
{
    m_acceptor = new tcp::acceptor(m_io_service, tcp::endpoint(tcp::v4(), m_port));
}

accept_manager::accept_manager(unsigned short port)
{
    m_port = port;
	m_test = 0;
	accept_manager();
}

accept_manager::~accept_manager()
{
    Stop();
    delete m_acceptor;
}

int accept_manager::Proc()
{
	tcp::socket *sock = new boost::asio::ip::tcp::socket(m_io_service);
	boost::asio::ip::tcp::no_delay option(true);
	boost::system::error_code ec;
	sock->set_option(option, ec);
	if (ec)
	{
	}
	m_acceptor->accept(*sock);
	bool auth = AuthConnect(sock);

	printf("STATUS SUCCESS\n");
	/*if(m_test<3) //for modifty database count 
	{
		
		printf("COUNT %d\n" , m_test);

		agent_client* new_client = new agent_client(sock);
		((agent_broker*)m_arg)->AddClient(new_client);
		cout<<"NEW CLINET CONNECT"<<endl;
		m_test++; //for test 
	}
	else
	{
		sock->close();
		delete sock;
		cout<< "NEW CLIENT CONNECT MAX COUNT IS OVER" <<endl;
		m_test--; //for test 
	}*/


	if(auth == true)
	{
		printf("COUNT %d\n" , m_test);

		agent_client* new_client = new agent_client(sock);
		((agent_broker*)m_arg)->AddClient(new_client);
		cout<<"NEW CLINET CONNECT\n"<<endl;
	}
	else
	{
		sock->close();
		delete sock;
		cout<< "NEW CLIENT CONNECT MAX COUNT IS OVER" <<endl;
		m_test--; //for test 
	}
	return 0;
}



bool accept_manager::AuthConnect(tcp::socket* _socket)
{
    size_t bytesReceived;
    boost::system::error_code error;
    bool check_status = false;
    int count =0;

    while(1)
    {
        if (m_recv_step == 0)
        {   
            while (m_prefix_buf.size() >= xpacket::PREFIX_SIZE)
            {   
                m_prefix_buf.pop_front();
            }
            
            bytesReceived = boost::asio::read(*_socket, boost::asio::buffer(m_recv_buf, 1));
            if(bytesReceived <=0)
            {   
                continue;
            }
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
                else
                {}
            }
        }
        if (m_recv_step == 1)//recv whole header
        {   
            int remain_header_size = xpacket::XPACKET_HEADER_SIZE - m_msg_buf_pos;
            bytesReceived = boost::asio::read(*_socket, boost::asio::buffer(m_recv_buf, remain_header_size));
            if(bytesReceived <=0)
            {   
                continue;
            }
            memcpy(m_msg_buf + m_msg_buf_pos, m_recv_buf, bytesReceived);
            m_msg_buf_pos += bytesReceived;
            
            if (m_msg_buf_pos >= xpacket::XPACKET_HEADER_SIZE)
            {   
                memcpy(&(m_making_packet->m_header), m_msg_buf, xpacket::XPACKET_HEADER_SIZE);
                m_recv_step = 2;
            }
            else
            {}
        }
        if (m_recv_step == 2)//recv body && tail
        {
            int remain_packet_size = m_making_packet->m_header.h_packet_size - m_msg_buf_pos;
            if (remain_packet_size > RECV_BUF_LEN)
            {//error
            }
            else
            {
                bytesReceived = boost::asio::read(*_socket, boost::asio::buffer(m_recv_buf, remain_packet_size));
                if(bytesReceived <=0)
                {
                    continue;
                }

                bytesReceived = remain_packet_size;
                memcpy(m_msg_buf + m_msg_buf_pos, m_recv_buf, bytesReceived);
                m_msg_buf_pos += bytesReceived;

                if (m_msg_buf_pos >= m_making_packet->m_header.h_packet_size)//complete
                {
                    if (m_msg_buf[m_msg_buf_pos - 2] == 0xFE && m_msg_buf[m_msg_buf_pos - 1] == 0xFF)
                    {
                        if (m_making_packet->m_header.h_packet_size > xpacket::XPACKET_HEADER_SIZE + xpacket::XPACKET_TAIL_SIZE)
                            m_making_packet->PushMem(m_msg_buf + xpacket::XPACKET_HEADER_SIZE, m_making_packet->m_header.h_packet_size - xpacket::XPACKET_HEADER_SIZE - xpacket::XPACKET_TAIL_SIZE);
                        m_making_packet->Pack();
                        //check_status=AuthCheckDB(m_making_packet,_socket);
                        check_status = true; // for test 
						m_making_packet = nullptr;
                    }
                    else //Wrong tail
                    {
                    }
                    m_msg_buf_pos = 0;
                    m_recv_step = 0;
                }
                count++;
            }
        }
        if(count == 1)
        {
            count = 0;
            break;
      }
    }
    return check_status;
}

