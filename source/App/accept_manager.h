#include "xpacket.h"
#include "xthread.h"
#include <utility>
#include <deque>
#include <boost/asio.hpp>

using boost::asio::ip::tcp;
class accept_manager : public xthread
{
    public:
        accept_manager();
        accept_manager(unsigned short port);
        virtual ~accept_manager();

        const static unsigned short DEFAULT_PORT = 6885;
        const static int RECV_BUF_LEN = 256;
        int m_test;
    private:
        virtual int Proc();
        bool AuthConnect(tcp::socket* _socket);
		unsigned short m_port = DEFAULT_PORT;
        boost::asio::io_service m_io_service;
        tcp::acceptor *m_acceptor;
        mutex m_socket_lock;

        int m_recv_step = 0;
        deque<uint8_t> m_prefix_buf;
        uint8_t m_recv_buf[RECV_BUF_LEN], m_msg_buf[RECV_BUF_LEN];
        size_t  m_msg_buf_pos = 0;
        xpacket *m_making_packet = nullptr;
};

