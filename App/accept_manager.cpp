#include "accept_manager.h"
#include "agent_client.h"
#include <iostream>

using namespace std;
accept_manager::accept_manager()
{
    m_acceptor = new tcp::acceptor(m_io_service, tcp::endpoint(tcp::v4(), m_port));
}

accept_manager::accept_manager(unsigned short port)
{
    m_port = port;
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
    if(m_test<3)
    {
    agent_client* new_client = new agent_client(sock);
    //((zetavu*)m_arg)->AddClient(new_client);
    cout<<"NEW CLINET CONNECT"<<endl;
    }
    m_test++; //for test 
    return 0;
}



